/*================================================================
    * Copyright: 2020 John Jackson
    * simple_triangle

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

gs_command_buffer_t               cb      = {0};
gs_handle(gs_graphics_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_pipeline_t) pip     = {0};
gs_handle(gs_graphics_shader_t)   shader  = {0};
gs_handle(gs_graphics_buffer_t)   u_color = {0};
gs_handle(gs_graphics_buffer_t)   u_model = {0};

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec2 a_pos;\n"
"uniform mat4 u_model;\n"
"void main()\n"
"{\n"
"   gl_Position = u_model * vec4(a_pos, 0.0, 1.0);\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"out vec4 frag_color;\n"
"uniform vec3 u_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(u_color, 1.0);\n"
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

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = v_data,
            .size = sizeof(v_data)
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
            .name = "color_shader"
        }
    );

    u_color = gs_graphics_buffer_create (
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_UNIFORM,                                         // Type of buffer (uniform)
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_VEC3},    // Description of internal uniform data
            .size = sizeof(gs_graphics_uniform_desc_t),                                 // Size of uniform description (used for counts, if uniform block used)
            .name = "u_color"                                                           // Name of uniform (used for linkage)
        }
    );

    u_model = gs_graphics_buffer_create (
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_UNIFORM,                                         // Type of buffer (uniform)
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4},    // Description of internal uniform data
            .size = sizeof(gs_graphics_uniform_desc_t),                                 // Size of uniform description (used for counts, if uniform block used)
            .name = "u_model"                                                             // Name of uniform (used for linkage)
        }
    );

    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}
    };

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

    // Bindings for all of our vertex data, uniform buffers, etc.
    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_UNIFORM_BUFFER, .buffer = u_color, .data = &color},  // Bind uniform buffer along with uniform data
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_UNIFORM_BUFFER, .buffer = u_model, .data = &model}   // Bind uniform buffer along with uniform data
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, binds, sizeof(binds));
        gs_graphics_draw(&cb, 0, 3, 1);
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






