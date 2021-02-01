/*================================================================
    * Copyright: 2020 John Jackson
    * instanced_drawing

    The purpose of this example is to demonstrate how to create do 
    instanced drawing using gunslinger.

    Included: 
        * Construct vertex/instanced data buffers from user defined declarations
        * Construct shaders from source
        * Rendering via command buffers

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

gs_command_buffer_t               cb       = {0};
gs_handle(gs_graphics_buffer_t)   vbo      = {0};
gs_handle(gs_graphics_buffer_t)   inst_vbo = {0};
gs_handle(gs_graphics_pipeline_t) pip      = {0};
gs_handle(gs_graphics_shader_t)   shader   = {0};

#define ROW_COL_CT  10

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec3 a_color;\n"
"layout(location = 2) in vec2 a_offset;\n"
"out vec3 f_color;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a_pos + a_offset, 0.0, 1.0);\n"
"   f_color = a_color;\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"in vec3 f_color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(f_color, 1.0);\n"
"}";

gs_vec2 g_translations[100] = {0};

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();
    
    float v_data[] = {
        // positions     // colors
        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
        -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

        -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
         0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
         0.05f,  0.05f,  0.0f, 1.0f, 1.0f
    };

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
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

    // Construct instanced data buffer
    inst_vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
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
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .stride = 5 * sizeof(float), .offset = 0, .buffer_idx = 0},                  // Position
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .stride = 5 * sizeof(float), .offset = 2 * sizeof(float), .buffer_idx = 0},  // Color
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .stride = 2 * sizeof(float), .offset = 0, .divisor = 1, .buffer_idx = 1},    // Offset (stride of total index vertex data, divisor is 1 for instance iteration)
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
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Render pass action for clearing the screen
    gs_graphics_render_pass_action_t action = (gs_graphics_render_pass_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}};

    // Bindings for buffers (order needs to match vertex layout buffer index layout up above for pipeline)
    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = inst_vbo}
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, binds, sizeof(binds));
        gs_graphics_draw(&cb, 0, 6, 100);
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






