/*================================================================
    * Copyright: 2020 John Jackson
    * instanced_drawing

    The purpose of this example is to demonstrate how to create do 
    instanced drawing using gunslinger.

    Modified from: https://learnopengl.com/Advanced-OpenGL/Instancing

    Included: 
        * Construct vertex/instanced data buffers from user defined declarations
        * Construct shaders from source
        * Drawing instances
        * Rendering via command buffers

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

// All necessary graphics data for this example
#include "data.c"

gs_command_buffer_t                     cb       = {0};
gs_handle(gs_graphics_vertex_buffer_t)  vbo      = {0};
gs_handle(gs_graphics_vertex_buffer_t)  inst_vbo = {0};
gs_handle(gs_graphics_pipeline_t)       pip      = {0};
gs_handle(gs_graphics_shader_t)         shader   = {0};

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Translation data
    int32_t index = 0;
    float offset = 0.1f;
    for (int32_t y = -10; y < 10; y += 2)
    {
        for (int32_t x = -10; x < 10; x += 2)
        {
            g_translations[index].x = (float)x / 10.0f + offset;
            g_translations[index].y = (float)y / 10.0f + offset;
            index++;
        }
    }

    // Construct vertex buffer
    vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

    // Construct instanced data buffer
    inst_vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = &g_translations[0],
            .size = sizeof(g_translations)
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
            .name = "quad"
        }
    );

    // Here's where we actually let the pipeline know how view our vertex data that we'll bind.
    // Need to actually describe vertex strides/offsets/divisors for instanced data layouts.
    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos", .stride = 5 * sizeof(float), .offset = 0, .buffer_idx = 0},                  // Position
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_color", .stride = 5 * sizeof(float), .offset = 2 * sizeof(float), .buffer_idx = 0},  // Color
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_offset", .stride = 2 * sizeof(float), .offset = 0, .divisor = 1, .buffer_idx = 1},    // Offset (stride of total index vertex data, divisor is 1 for instance iteration)
    };

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader,
                .index_buffer_element_size = sizeof(uint32_t) 
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
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    // Clear description
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};

    // Bindings for buffers (order needs to match vertex layout buffer index layout up above for pipeline)
    gs_graphics_bind_vertex_buffer_desc_t vbos[] = {
        {.buffer = vbo},                        // Vertex buffer 0
        {.buffer = inst_vbo}                    // Vertex buffer 1
    };

    // Construct binds
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = vbos, .size = sizeof(vbos)}
    };

    /* Render */
    gs_graphics_renderpass_begin(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_apply_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6, .instances = 100});
    gs_graphics_renderpass_end(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






