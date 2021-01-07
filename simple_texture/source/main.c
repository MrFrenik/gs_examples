/*================================================================
    * Copyright: 2020 John Jackson
    * simple_texture

    The purpose of this example is to demonstrate how to create a 
    dynamic texture, load a texture from file, and explicitly construct 
    GPU resources to use for your application. Uses a shader to blend
    two textures together.

    Included: 
        * Constructing textures via tetxure despcriptors 
        * Construct vertex/index buffers from user defined declarations
        * Construct shaders from source
        * Construct sampler buffers for binding and uploading texture data to shader
        * Rendering via command buffers

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

gs_command_buffer_t               cb      = {0};
gs_handle(gs_graphics_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_buffer_t)   ibo     = {0};
gs_handle(gs_graphics_pipeline_t) pip     = {0};
gs_handle(gs_graphics_shader_t)   shader  = {0};
gs_handle(gs_graphics_buffer_t)   u_tex   = {0};
gs_handle(gs_graphics_texture_t)  tex     = {0};

gs_asset_texture_t texture = {0};

#define ROW_COL_CT  10

const char* v_src = "\n"
"#version 330 core\n"
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec2 a_uv;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"   uv = a_uv;\n"
"}";

const char* f_src = "\n"
"#version 330 core\n"
"uniform sampler2D u_tex;"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = texture(u_tex, uv);\n"
"}";

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new(); 

    // Generate procedural texture data (checkered texture)
    gs_color_t c0 = GS_COLOR_WHITE;
    gs_color_t c1 = gs_color(20, 50, 150, 255);
    gs_color_t pixels[ROW_COL_CT * ROW_COL_CT] = gs_default_val();
    for (uint32_t r = 0; r < ROW_COL_CT; ++r) {
        for (uint32_t c = 0; c < ROW_COL_CT; ++c) {
            const bool re = (r % 2) == 0;
            const bool ce = (c % 2) == 0;
            uint32_t idx = r * ROW_COL_CT + c;
            pixels[idx] = (re && ce) ? c0 : (re) ? c1 : (ce) ? c1 : c0;
        } 
    }

    // Create dynamic texture
    tex = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = ROW_COL_CT,
            .height = ROW_COL_CT,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .data = pixels
        }
    );

    gs_graphics_sampler_desc_t sdesc = {
        .type = GS_GRAPHICS_SAMPLER_2D
    };

    // Construct sampler buffer
    u_tex = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_SAMPLER,
            .data = &sdesc,
            .size = sizeof(sdesc),
            .name = "u_tex"
        }
    );
    
    // Vertex data for quad
    f32 v_data[] = 
    {
        // Positions  UVs
        -0.5f, -0.5f,  0.0f, 0.0f,  // Top Left
         0.5f, -0.5f,  1.0f, 0.0f,  // Top Right 
        -0.5f,  0.5f,  0.0f, 1.0f,  // Bottom Left
         0.5f,  0.5f,  1.0f, 1.0f   // Bottom Right
    };

    u32 i_data[] = 
    {
        0, 3, 2,    // First Triangle
        0, 1, 3     // Second Triangle
    };

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

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
            .name = "quad"
        }
    );

    gs_graphics_vertex_attribute_type layout[] = {
        GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,
        GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2
    };

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader,
                .index_buffer_element_size = sizeof(u32) 
            },
            .layout = layout,
            .size = sizeof(layout)
        }
    );
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Render pass action for clearing the screen
    gs_graphics_render_pass_action_t action = (gs_graphics_render_pass_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}};

    // Bindings for all of our vertex data, uniform buffers, etc.
    gs_graphics_bind_desc_t binds[] = {
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_VERTEX_BUFFER, .buffer = vbo},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_INDEX_BUFFER, .buffer = ibo},
        (gs_graphics_bind_desc_t){.type = GS_GRAPHICS_BIND_SAMPLER_BUFFER, .buffer = u_tex, .data = &tex, .binding = 0}
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, binds, sizeof(binds));
        gs_graphics_draw(&cb, 0, 6);
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






