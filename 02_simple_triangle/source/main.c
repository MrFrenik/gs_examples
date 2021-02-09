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

gs_command_buffer_t               cb      = {0};
gs_handle(gs_graphics_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_pipeline_t) pip     = {0};
gs_handle(gs_graphics_shader_t)   shader  = {0};

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec2 a_pos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}";

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new(); 

    // Vertex data for triangle
    float v_data[] = {
        0.0f, 0.5f,
        -0.5f, -0.5f, 
        0.5f, -0.5f
    };

// typedef struct gs_graphics_buffer_desc_t 
// {
//     union {
//         struct {
//             void* data;
//             size_t size;
//             const char* name;
//         } vertex_buffer;
//         struct {
//             void* data;
//             size_t size;
//             const char* name;
//         } index_buffer;
//         struct {
//             gs_graphics_shader_stage_type shader_stage;
//             const char* name;
//             gs_graphics_uniform_type* layout;
//             size_t layout_size;
//         } uniform_constant;
//         struct {
//             gs_graphics_shader_stage_type shader_stage;
//             const char* name;
//             void* data;
//             size_t size;
//         } uniform_buffer;
//         struct {
//             gs_graphics_shader_stage_type shader_stage;
//             gs_graphics_sampler_type type;
//             const char* name; 
//         } sampler_buffer;
//     };
// } gs_graphics_buffer_desc_t;

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .vertex_buffer = {
                .data = v_data,
                .size = sizeof(v_data)
            }
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
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}
                },
                .size = sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Render pass action for clearing the screen
    gs_graphics_render_pass_action_t action = (gs_graphics_render_pass_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}};

    // Binding descriptor for vertex buffer
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.decl = &(gs_graphics_bind_buffer_desc_t){.buffer = vbo}}
    };

    // Begin render pass (default render pass draws to back buffer)
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT, &action, sizeof(action));
        // Bind our triangle pipeline for rendering
        gs_graphics_bind_pipeline(&cb, pip);
        // Bind all bindings (just vertex buffer)
        gs_graphics_bind_bindings(&cb, &binds);
        // Draw the triangle
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 3});
    // End the render pass
    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






