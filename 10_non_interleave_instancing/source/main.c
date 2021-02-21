/*================================================================
    * Copyright: 2020 John Jackson
    * non_interleave_instancing

    The purpose of this example is to demonstrate how to use both 
    non-interleaved data with instanced drawing in gunslinger.

    Thanks to discord user 'chillagen' for the initial example idea.

    Included: 
        * Construct vertex/instanced data buffers from user defined declarations
        * Update vertex buffer subregions using separate data arrays
        * Drawing instances
        * Rendering via command buffers

    Controls: 
        * Hold left mouse button to activate fly camera
        * Mouse to rotate camera view
        * WASD to move camera

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#include "data.c"

void fps_camera_update(gs_camera_t* cam);

gs_command_buffer_t                     cb       = {0};
gs_handle(gs_graphics_vertex_buffer_t)  vbo      = {0};
gs_handle(gs_graphics_index_buffer_t)   ibo      = {0};
gs_handle(gs_graphics_uniform_t)        u_mvp    = {0};
gs_handle(gs_graphics_pipeline_t)       pip      = {0};
gs_handle(gs_graphics_shader_t)         shader   = {0};
gs_handle(gs_graphics_vertex_buffer_t)  inst_vbo = {0};
gs_camera_t                             cam      = {0};

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Construct camera
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    cam = gs_camera_perspective();
    cam.transform.position = gs_v3(-15.89f, 4.45f, -0.08f);
    cam.transform.rotation = gs_quat(0.02f, -0.79f, 0.02f, 0.61f);

    for (int32_t i = 0; i < NUM_OFFSETS; ++i) {

        offsets[i].x = 4000.0f * stb_frand() -2000.0f;
        offsets[i].y = 4000.0f * stb_frand() -2000.0f;
        offsets[i].z = 4000.0f * stb_frand() -2000.0f;
    }

    // Construct vertex buffer
    vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = NULL,
            .size = sizeof(positions) + sizeof(colors)  // Total size for buffer data to be used
        }
    );

    // Upload data for positions
    gs_graphics_vertex_buffer_request_update(&cb, vbo,
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = positions,
            .size = sizeof(positions),
            .update = {
                .offset = 0,
                .type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
            }
        }
    );

    // Upload data for colors
    gs_graphics_vertex_buffer_request_update(&cb, vbo,
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = colors,
            .size = sizeof(colors),
            .update = {
                .offset = sizeof(positions),
                .type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
            }
        }
    );

    inst_vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = offsets,
            .size = sizeof(offsets)       
        }
    );

    // Construct index buffer
    ibo = gs_graphics_index_buffer_create( 
        &(gs_graphics_index_buffer_desc_t) {
            .data = i_data, 
            .size = sizeof(i_data)
        }
    );

    // Shader source description
    gs_graphics_shader_source_desc_t sources[] = {
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
    };

    // Create shader
    shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = sources, 
            .size = sizeof(sources),
            .name = "cube"
        }
    );

    u_mvp = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_mvp",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    // Here's where we actually let the pipeline know how view our vertex data that we'll bind.
    // Need to actually describe vertex strides/offsets/divisors for instanced data layouts.
    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_pos", .buffer_idx = 0}, // Position
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4, .name = "a_color", .buffer_idx = 1}, // Color
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "i_offset", .buffer_idx = 2, .stride = 3 * sizeof(float), .offset = 0, .divisor = 1},    // Offset (stride of total index vertex data, divisor is 1 for instance iteration)
    };

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader,
                .index_buffer_element_size = sizeof(uint16_t) 
            },
            .depth = {
                .func = GS_GRAPHICS_DEPTH_FUNC_LESS
            },
            .layout = {
                .attrs = vattrs,
                .size = sizeof(vattrs)
            }
        }
    );
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) 
        gs_engine_quit();

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Update camera
    fps_camera_update(&cam);

    // Calculate mvp matrix
    gs_mat4 mvp = gs_camera_get_view_projection(&cam, (int32_t)ws.x ,(int32_t)ws.y);

    // Declare all binds
    gs_graphics_bind_vertex_buffer_desc_t vbuffers[] = {
        {.buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0},                  // Vertex Buffer Idx 0
        {.buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = sizeof(positions)},  // Vertex Buffer Idx 1
        {.buffer = inst_vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0}              // Vertex Buffer Idx 2
    };

    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = vbuffers, .size = sizeof(vbuffers)},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = ibo}},
        .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = u_mvp, .data = &mvp}}
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_apply_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36, .instances = NUM_OFFSETS});
    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

void fps_camera_update(gs_camera_t* cam)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);

    gs_platform_lock_mouse(gs_platform_main_window(), gs_platform_mouse_down(GS_MOUSE_LBUTTON));
    if (!gs_platform_mouse_locked()) {
        return;
    }

    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    const gs_vec2 dp = gs_platform_mouse_deltav();
    const float mod = gs_platform_key_down(GS_KEYCODE_LEFT_SHIFT) ? 5.f : 1.f; 
    float dt = platform->time.delta;

    // Rotate camera
    gs_camera_offset_orientation(cam, -dp.x, -dp.y);

    gs_vec3 vel = {0};
    if (gs_platform_key_down(GS_KEYCODE_W)) vel = gs_vec3_add(vel, gs_camera_forward(cam));
    if (gs_platform_key_down(GS_KEYCODE_S)) vel = gs_vec3_add(vel, gs_camera_backward(cam));
    if (gs_platform_key_down(GS_KEYCODE_A)) vel = gs_vec3_add(vel, gs_camera_left(cam));
    if (gs_platform_key_down(GS_KEYCODE_D)) vel = gs_vec3_add(vel, gs_camera_right(cam));

    cam->transform.position = gs_vec3_add(cam->transform.position, gs_vec3_scale(gs_vec3_norm(vel), dt * CAM_SPEED * mod));
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}   






