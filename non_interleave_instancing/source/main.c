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
#include <stdio.h>

#define NUM_OFFSETS 1000000         // One MILLION offsets! Mwahahaha.

gs_command_buffer_t               cb      = {0};
gs_handle(gs_graphics_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_buffer_t)   ibo     = {0};
gs_handle(gs_graphics_buffer_t)   u_mvp   = {0};
gs_handle(gs_graphics_pipeline_t) pip     = {0};
gs_handle(gs_graphics_shader_t)   shader  = {0};
gs_handle(gs_graphics_buffer_t)   inst_vbo = {0};
gs_camera_t camera;

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;\n"
"layout(location = 1) in vec4 a_color;\n"
"layout(location = 2) in vec3 i_position;\n"
"uniform mat4 u_mvp;\n"
"out vec4 f_color;\n"
"void main()\n"
"{\n"
"   gl_Position =u_mvp * vec4(a_pos + i_position, 1.0);\n"
"   f_color = a_color;\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"in vec4 f_color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = f_color;\n"
"}";

// Cube vertex buffer
gs_global float positions[] = {
    -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0,
    -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0,
    -1.0, -1.0, -1.0,  -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,  -1.0, -1.0,  1.0,
     1.0, -1.0, -1.0,   1.0,  1.0, -1.0,   1.0,  1.0,  1.0,   1.0, -1.0,  1.0, 
    -1.0, -1.0, -1.0,  -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,   1.0,  1.0,  1.0,   1.0,  1.0, -1.0,
};

gs_global float colors[] = {
    1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,
    0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,
    0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,
    1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,
    0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,
    1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,
};


gs_vec3 offsets[NUM_OFFSETS         // One MILLION offsets! Mwahahaha.];


void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Construct camera
    camera = gs_camera_perspective();

    // Set up camera to a pleasing starting position/orientation
    camera.transform.position = gs_v3(-15.89f, 4.45f, -0.08f);
    camera.transform.rotation = gs_quat(0.02f, -0.79f, 0.02f, 0.61f);
    

    for (int32_t i =            // One MILLION offsets! Mwahahaha. 0; i < NUM_OFFSETS; ++i) {

        offsets[i].x = 10.0f * i;
        offsets[i].y = 10.0f * i;
        offsets[i].z = 10.0f * i;
    }

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = NULL,
            .size = sizeof(positions) + sizeof(colors)  // Total size for buffer data to be used
        }
    );

    // Upload data for positions
    gs_graphics_buffer_request_update(
        &cb,
        vbo,
        &(gs_graphics_buffer_desc_t) {
            .usage = GS_GRAPHICS_BUFFER_USAGE_STATIC,
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = positions,
            .size = sizeof(positions),
            .update = {
                .offset = 0,
                .type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
            }
        }
    );

    // Upload data for colors
    gs_graphics_buffer_request_update(
        &cb,
        vbo,
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = colors,
            .size = sizeof(colors),
            .update = {
                .offset = sizeof(positions),
                .type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA
            }
        }
    );

    inst_vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = offsets,
            .size = sizeof(offsets),         
        }
    );

    // Index data
    uint16_t i_data[] = {
        0, 1, 2,  0, 2, 3,
        6, 5, 4,  7, 6, 4,
        8, 9, 10,  8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    }; 

    // Construct index buffer
    ibo = gs_graphics_buffer_create( 
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_INDEX,
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

    // Create mvp uniform buffer
    u_mvp = gs_graphics_buffer_create (
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_UNIFORM,                                        // Type of buffer (uniform)
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4},   // Description of internal uniform data
            .size = sizeof(gs_graphics_uniform_desc_t),                                // Size of uniform description (used for counts, if uniform block used)
            .name = "u_mvp"                                                            // Name of uniform (used for linkage)
        }
    );

    // Here's where we actually let the pipeline know how view our vertex data that we'll bind.
    // Need to actually describe vertex strides/offsets/divisors for instanced data layouts.
    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .buffer_idx = 0}, // Position
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4, .buffer_idx = 1}, // Color
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .buffer_idx = 2, .stride = 3 * sizeof(float), .offset = 0, .divisor = 1},    // Offset (stride of total index vertex data, divisor is 1 for instance iteration)
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

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Render pass action for clearing the screen
    gs_graphics_render_pass_action_t action = (gs_graphics_render_pass_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}};
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    const gs_vec2 mouse_delta = gs_platform_mouse_deltav();

    if (gs_platform_mouse_down(GS_MOUSE_LBUTTON)) {
        gs_camera_offset_orientation(&camera, -mouse_delta.x, -mouse_delta.y);
    }

    if (gs_platform_key_down(GS_KEYCODE_W)) {
        camera.transform.position = gs_vec3_add(gs_camera_forward(&camera), camera.transform.position);
    }
    if (gs_platform_key_down(GS_KEYCODE_S)) {
        camera.transform.position = gs_vec3_add(gs_camera_backward(&camera), camera.transform.position);
    }
    if (gs_platform_key_down(GS_KEYCODE_A)) {
        camera.transform.position = gs_vec3_add(gs_camera_left(&camera), camera.transform.position);
    }
    if (gs_platform_key_down(GS_KEYCODE_D)) {
        camera.transform.position = gs_vec3_add(gs_camera_right(&camera), camera.transform.position);
    }
    
    gs_mat4 mvp = gs_camera_get_view_projection(&camera, (int32_t)ws.x ,(int32_t)ws.y);
    // Bindings for buffers (order needs to match vertex layout buffer index layout up above for pipeline)
    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0},                  // Positions
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = sizeof(positions)},  // Colors
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = inst_vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0},             // Instance Offsets
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_UNIFORM_BUFFER, .buffer = u_mvp, .data = &mvp},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_INDEX_BUFFER, .buffer = ibo},
        
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, binds, sizeof(binds));

        gs_graphics_draw(&cb,           // One MILLION offsets! Mwahahaha. 0, 36, NUM_OFFSETS);

    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update,
        .enable_vsync=1,
    };
}   






