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

gs_command_buffer_t               cb      = {0};
gs_handle(gs_graphics_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_buffer_t)   ibo     = {0};
gs_handle(gs_graphics_buffer_t)   u_mvp   = {0};
gs_handle(gs_graphics_pipeline_t) pip     = {0};
gs_handle(gs_graphics_shader_t)   shader  = {0};

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec3 a_pos;\n"
"layout(location = 1) in vec4 a_color;\n"
"uniform mat4 u_mvp;\n"
"out vec4 f_color;\n"
"void main()\n"
"{\n"
"   gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
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

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();
    
    // Cube vertex buffer
    float v_data[] = {
        // Positions
        -1.0, -1.0, -1.0,   1.0, -1.0, -1.0,   1.0,  1.0, -1.0,  -1.0,  1.0, -1.0,
        -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0,  1.0,  1.0,  -1.0,  1.0,  1.0,
        -1.0, -1.0, -1.0,  -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,  -1.0, -1.0,  1.0,
         1.0, -1.0, -1.0,   1.0,  1.0, -1.0,   1.0,  1.0,  1.0,   1.0, -1.0,  1.0, 
        -1.0, -1.0, -1.0,  -1.0, -1.0,  1.0,   1.0, -1.0,  1.0,   1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,  -1.0,  1.0,  1.0,   1.0,  1.0,  1.0,   1.0,  1.0, -1.0,

        // Colors
        1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,  1.0, 0.5, 0.0, 1.0,
        0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,  0.5, 1.0, 0.0, 1.0,
        0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,  0.5, 0.0, 1.0, 1.0,
        1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,  1.0, 0.5, 1.0, 1.0,
        0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,  0.5, 1.0, 1.0, 1.0,
        1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,  1.0, 1.0, 0.5, 1.0,
    };

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = v_data,
            .size = sizeof(v_data)
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
    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

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

    // Bindings for buffers (order needs to match vertex layout buffer index layout up above for pipeline)
    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 0},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo, .data_type = GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED, .offset = 24 * 3 * sizeof(float)},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_INDEX_BUFFER, .buffer = ibo},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_UNIFORM_BUFFER, .buffer = u_mvp, .data = &mvp}
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, binds, sizeof(binds));
        gs_graphics_draw(&cb, 0, 36, 1);
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






