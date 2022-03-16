/*================================================================
    * Copyright: 2020 John Jackson
    * non_interleaved_data

    The purpose of this example is to demonstrate how to create vertex
    data that's non-interleaved.

    Modified from: https://github.com/floooh/sokol-samples/blob/master/glfw/noninterleaved-glfw.c

    Included: 
        * Construct non-interleaved vertex data buffers from user defined declarations
        * Construct shaders from source
        * Rendering via command buffers

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

// All necessary graphics data for this example
#include "data.c"

gs_command_buffer_t                     cb      = {0};
gs_handle(gs_graphics_vertex_buffer_t)  vbo     = {0};
gs_handle(gs_graphics_index_buffer_t)   ibo     = {0};
gs_handle(gs_graphics_uniform_t)        u_mvp   = {0};
gs_handle(gs_graphics_pipeline_t)       pip     = {0};
gs_handle(gs_graphics_shader_t)         shader  = {0};

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Construct vertex buffer
    vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = v_data,
            .size = sizeof(v_data)
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

    // Create mvp uniform buffer
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
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // MVP Matrix
    gs_mat4 mvp = gs_mat4_perspective(60.f, ws.x / ws.y, 0.1f, 100.f);
    mvp = gs_mat4_mul_list(
        6, 
        mvp,
        gs_mat4_translate(0.f, 0.f, -2.f),
        gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0001f, GS_YAXIS), 
        gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0005f, GS_XAXIS), 
        gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0002f, GS_ZAXIS), 
        gs_mat4_scale(0.5f, 0.5f, 0.5f)
    );

    // Declare all binds
    gs_graphics_bind_vertex_buffer_desc_t vbos[] = {
        {.buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0},                      // Vertex Buffer Idx 0
        {.buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 24 * 3 * sizeof(float)}, // Vertex Buffer Idx 1
    };

    // Binding descriptor
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = vbos, .size = sizeof(vbos)},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = ibo}},
        .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = u_mvp, .data = &mvp}}
    };

    /* Render */
    gs_graphics_renderpass_begin(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_pipeline_bind(&cb, pip);
        gs_graphics_apply_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
    gs_graphics_renderpass_end(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}   






