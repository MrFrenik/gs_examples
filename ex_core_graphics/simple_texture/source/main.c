/*================================================================
    * Copyright: 2020 John Jackson
    * simple_texture

    The purpose of this example is to demonstrate how to create a 
    dynamic texture and explicitly construct GPU resources to use 
    for your application.

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

// All necessary graphics data for this example
#include "data.c"

gs_command_buffer_t                      cb      = {0};
gs_handle(gs_graphics_vertex_buffer_t)   vbo     = {0};
gs_handle(gs_graphics_index_buffer_t)    ibo     = {0};
gs_handle(gs_graphics_pipeline_t)        pip     = {0};
gs_handle(gs_graphics_shader_t)          shader  = {0};
gs_handle(gs_graphics_uniform_t)         u_tex   = {0};
gs_handle(gs_graphics_texture_t)         tex     = {0};

gs_asset_texture_t texture = {0};

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

    int32_t width, height, num_comps;
    void* data = NULL;
    gs_util_load_texture_data_from_file("./assets/champ.png", &width, &height, &num_comps, &data, true);

    // Create dynamic texture
    tex = gs_graphics_texture_create (
        &(gs_graphics_texture_desc_t){
            .width = width,
            .height = height,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .data = data
        }
    );

    // Construct sampler buffer
    u_tex = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "u_tex",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}
        }
    );

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

    // Create shader
    shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]){
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
            }, 
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "quad"
        }
    );

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader,
                .index_buffer_element_size = sizeof(uint32_t)
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_pos"},
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "a_uv"}
                },
                .size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t){.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};

    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    // Bindings for all buffers: vertex, index, sampler
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vbo}},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = ibo}},
        .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = u_tex, .data = &tex, .binding = 0}}
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_apply_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
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






