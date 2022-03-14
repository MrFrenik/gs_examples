/*================================================================
    * Copyright: 2020 John Jackson
    * uniforms

    The purpose of this example is to demonstrate how to bind and upload 
    uniform data to the GPU using pipelines and uniform buffers.

    Included: 
        * Construct vertex/index buffers from user defined declarations
        * Construct shaders from source
        * Construct uniforms and bind data to pass to GPU
        * Construct pipelines
        * Rendering via command buffers

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#include "data.c"

gs_command_buffer_t                      cb      = {0};
gs_handle(gs_graphics_vertex_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_pipeline_t)        pip     = {0};
gs_handle(gs_graphics_shader_t)          shader  = {0};
gs_handle(gs_graphics_uniform_t)         u_color = {0};
gs_handle(gs_graphics_uniform_t)         u_model = {0};

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
            .sources = (gs_graphics_shader_source_desc_t[]){
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src},
            },
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "color_shader"
        }
    );

    u_color = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_color",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC3}
        }
    );

    u_model = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_model",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader
            },
            .blend = {
                .func = GS_GRAPHICS_BLEND_EQUATION_ADD,
                .src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
                .dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA              
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}
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

    // Uniform data to pass up to our shader (make some random colors change over time)
    const float t = gs_platform_elapsed_time() * 0.0001f;
    const float r = sin(t) * 0.5f + 0.5f; 
    const float g = cos(t * 6.f) * 0.5f + 0.5f; 
    const float b = sin(t * 3.f) * 0.5f + 0.5f; 
    gs_vec3 color = gs_v3(r, g, b);

    // Uniform data for model matrix (just a rotation matrix over time around z axis)
    const float st = sin(t);
    gs_mat4 rot = gs_mat4_rotatev(t, GS_ZAXIS);
    gs_mat4 scl = gs_mat4_scalev(gs_v3(st, st, st));
    gs_mat4 model = gs_mat4_mul_list(2, scl, rot);

    // Uniform buffer array
    gs_graphics_bind_uniform_desc_t uniforms[] = {
        (gs_graphics_bind_uniform_desc_t){.uniform = u_color, .data = &color},
        (gs_graphics_bind_uniform_desc_t){.uniform = u_model, .data = &model},
    };

    // Binding descriptor for vertex buffer
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vbo}},
        .uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
    };

    /* Render */
    gs_graphics_renderpass_begin(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_apply_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 3});
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






