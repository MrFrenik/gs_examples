/*================================================================
    * Copyright: 2020 John Jackson
    * simple_triangle

    The purpose of this example is to demonstrate how to explicitly construct 
    GPU resources to use for your application to render a basic triangle.

    Included: 
        * Construct vertex/index buffers from user defined declarations
        * Construct shaders from source
        * Construct pipelines
        * Rendering via command buffers

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

// All necessary graphics data for this example (shader source/vertex data)
#include "data.c"

gs_command_buffer_t                      cb      = {0};
gs_handle(gs_graphics_vertex_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_pipeline_t)        pip     = {0};
gs_handle(gs_graphics_shader_t)          shader  = {0};

void init()
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

    // Create shader
    shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]) {
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
            }, 
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "triangle"
        }
    );

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"}    // Named attribute required for lower GL versions / ES2 / ES3
                },
                .size = sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t){
        .actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}}
    };

    // Binding descriptor for vertex buffer
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {&(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vbo}},
    };

    gs_graphics_renderpass_begin(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);            // Begin render pass (default render pass draws to back buffer)
        gs_graphics_clear(&cb, &clear);                                            // Clear screen
        gs_graphics_bind_pipeline(&cb, pip);                                       // Bind our triangle pipeline for rendering
        gs_graphics_apply_bindings(&cb, &binds);                                   // Bind all bindings (just vertex buffer)
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 3}); // Draw the triangle
    gs_graphics_renderpass_end(&cb);                                               // End the render pass

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






