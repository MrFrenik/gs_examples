/*================================================================
    * Copyright: 2020 John Jackson
    * stencil_test

    Show how to use stencil testing in gunslinger.

    Modified from: https://learnopengl.com/Advanced-OpenGL/Stencil-testing

    Move around with camera: 
        * Left-click and hold to rotate camera view.
        * WASD to move while holding mouse.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

// Include all necessary data for program
#include "data.c"

typedef struct fps_camera_t
{
    gs_camera_t camera;
    gs_vec2 prev_mouse_position;
} fps_camera_t;

void fps_camera_update(fps_camera_t* cam);

gs_command_buffer_t                      cb         = {0};
fps_camera_t                             fps        = {0};
gs_handle(gs_graphics_vertex_buffer_t)   cvbo       = {0};
gs_handle(gs_graphics_vertex_buffer_t)   pvbo       = {0};
gs_handle(gs_graphics_pipeline_t)        pips[3]    = {0};
gs_handle(gs_graphics_shader_t)          shaders[2] = {0};
gs_handle(gs_graphics_texture_t)         tex0       = {0};
gs_handle(gs_graphics_texture_t)         tex1       = {0};
gs_handle(gs_graphics_uniform_t)         u_model    = {0};
gs_handle(gs_graphics_uniform_t)         u_vp       = {0};
gs_handle(gs_graphics_uniform_t)         u_tex      = {0};

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Set up camera
    fps.camera = gs_camera_perspective();
    fps.camera.transform.position = gs_v3(0.f, 0.f, 3.f);

    // Construct vertex buffer (cube)
    cvbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = cube_vdata,
            .size = sizeof(cube_vdata)
        }
    );

    // Construct vertex buffer (plane)
    pvbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = plane_vdata,
            .size = sizeof(plane_vdata)
        }
    );

    // Texture data information
    gs_color_t c0 = (gs_color_t){255, 255, 255, 255};
    gs_color_t c1 = (gs_color_t){20, 50, 150, 255};
    gs_color_t c2 = (gs_color_t){150, 200, 30, 255};
    gs_color_t pixels[ROW_COL_CT * ROW_COL_CT] = gs_default_val();

    // Generate pixel data for tex0
    generate_texture(pixels, c0, c1);

    // Create diffuse texture
    tex0 = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = ROW_COL_CT,
            .height = ROW_COL_CT,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
            .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .data = pixels
        }
    );

    // Generate pixel data for tex1
    generate_texture(pixels, c0, c2);

    tex1 = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = ROW_COL_CT,
            .height = ROW_COL_CT,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .data = pixels
        }
    );

    // Construct individual uniforms
    u_model = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_model",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    u_vp = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_vp",
            .layout = (gs_graphics_uniform_layout_desc_t[]){
                {.type = GS_GRAPHICS_UNIFORM_MAT4, .fname = ".proj"},
                {.type = GS_GRAPHICS_UNIFORM_MAT4, .fname = ".view"},
            },
            .layout_size = 2 * sizeof(gs_graphics_uniform_layout_desc_t)
        }
    );

    u_tex = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_tex",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}}
        }
    );

    const char* f_srcs[] = {
        f_texsrc,
        f_colsrc
    };

    // Create shaders
    for (uint32_t i = 0; i < sizeof(f_srcs) / sizeof(const char*); ++i) {
        shaders[i] = gs_graphics_shader_create (
            &(gs_graphics_shader_desc_t) {
                .sources = (gs_graphics_shader_source_desc_t[]){
                    {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                    {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_srcs[i]}
                },
                .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
                .name = "shader"
            }
        );
    }

    gs_graphics_stencil_state_desc_t stencils[] = {
        (gs_graphics_stencil_state_desc_t){
            .func = GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL,
            .ref = 1,
            .comp_mask = 0xFF,
            .write_mask = 0x00
        },
        (gs_graphics_stencil_state_desc_t){
            .func = GS_GRAPHICS_STENCIL_FUNC_ALWAYS,
            .ref = 1,
            .comp_mask = 0xFF,
            .write_mask = 0xFF,
            .sfail = GS_GRAPHICS_STENCIL_OP_KEEP,
            .dpfail = GS_GRAPHICS_STENCIL_OP_KEEP,
            .dppass = GS_GRAPHICS_STENCIL_OP_REPLACE
        },
        (gs_graphics_stencil_state_desc_t){
            .func = GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL,
            .ref = 1,
            .comp_mask = 0xFF,
            .write_mask = 0xFF
        }
    };

    gs_graphics_depth_func_type depth_funcs[] = {
        GS_GRAPHICS_DEPTH_FUNC_LESS,
        GS_GRAPHICS_DEPTH_FUNC_LESS,
        0x00
    };

    gs_handle(gs_graphics_shader_t) srefs[] = {
        shaders[0],
        shaders[0],
        shaders[1]
    };

    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3},  // Position
        {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}   // TexCoord
    };

    // Create all pipelines needed
    for (uint32_t i = 0; i < 3; ++i)
    {
        pips[i] = gs_graphics_pipeline_create (
            &(gs_graphics_pipeline_desc_t) {
                .raster = {
                    .shader = srefs[i]
                },
                .depth = {
                    .func = depth_funcs[i]
                },
                .stencil = stencils[i],
                .layout = {
                    .attrs = vattrs,
                    .size = sizeof(vattrs)
                }
            }
        );
    }
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    fps_camera_update(&fps);

    gs_vec2 fs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Action for clearing the screen
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}}};

    // Projection/View matrices
    gs_mat4 vp[] = {
        gs_camera_get_proj(&fps.camera, (int32_t)ws.x, (int32_t)ws.y),
        gs_camera_get_view(&fps.camera)
    };

    gs_mat4 translations[] = {
        gs_mat4_translate(-1.f, 0.f, -1.f),
        gs_mat4_translate(2.f, 0.f, 0.f)
    };

    gs_mat4 model = gs_mat4_identity();

    // Uniform bindings that don't change per object
    gs_graphics_bind_desc_t vp_binds = {.uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t) {.uniform = u_vp, .data = vp}}};
    gs_graphics_bind_desc_t model_binds = gs_default_val();

    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT); {
        gs_graphics_set_viewport(&cb, 0, 0, (uint32_t)fs.x, (uint32_t)fs.y);
        gs_graphics_clear(&cb, &clear);

        // 1). Draw floor as normal, but don't write the floor to the stencil buffer, we only care about the containers. 
        //     We set its mask to 0x00 to not write to the stencil buffer.        
        {
            gs_graphics_bind_pipeline(&cb, pips[0]);
            model_binds = (gs_graphics_bind_desc_t){
                .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = pvbo}},
                .uniforms = {
                    .desc = (gs_graphics_bind_uniform_desc_t[]){
                        {.uniform = u_model, .data = &model},
                        {.uniform = u_tex, .data = &tex0}
                    },
                    .size = 2 * sizeof(gs_graphics_bind_uniform_desc_t)
                } 
            };
            gs_graphics_apply_bindings(&cb, &vp_binds);
            gs_graphics_apply_bindings(&cb, &model_binds);
            gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 6});
        }

        // 2). 1st. render pass, draw objects as normal, writing to the stencil buffer
        {
            for (uint32_t i = 0; i < sizeof(translations) / sizeof(gs_mat4); ++i) {
                gs_graphics_bind_pipeline(&cb, pips[1]);
                model_binds = (gs_graphics_bind_desc_t){
                    .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = cvbo}},
                    .uniforms = {
                        .desc = (gs_graphics_bind_uniform_desc_t[]){
                            {.uniform = u_model, .data = &translations[i]},
                            {.uniform = u_tex, .data = &tex1}
                        },
                        .size = 2 * sizeof(gs_graphics_bind_uniform_desc_t)
                    } 
                };
                gs_graphics_apply_bindings(&cb, &vp_binds);
                gs_graphics_apply_bindings(&cb, &model_binds);
                gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
            }
        }

        // 3). 2nd. render pass: now draw slightly scaled versions of the objects, this time disabling stencil writing.
        // Because the stencil buffer is now filled with several 1s. The parts of the buffer that are 1 are not drawn, thus only drawing 
        // the objects' size differences, making it look like borders.0
        {
            for (uint32_t i = 0; i < sizeof(translations) / sizeof(gs_mat4); ++i) {
                const float scale = 1.1f;
                model = gs_mat4_mul(translations[i], gs_mat4_scale(scale, scale, scale));
                gs_graphics_bind_pipeline(&cb, pips[2]);
                model_binds = (gs_graphics_bind_desc_t){
                    .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = cvbo}},
                    .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = u_model, .data = &model}}
                };
                gs_graphics_apply_bindings(&cb, &vp_binds);
                gs_graphics_apply_bindings(&cb, &model_binds);
                gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
            }
        }
    }
    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

void fps_camera_update(fps_camera_t* fps)
{
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    const gs_vec2 mp = gs_platform_mouse_positionv();
    float dt = gs_subsystem(platform)->time.delta;

    // First pressed
    if (gs_platform_mouse_pressed(GS_MOUSE_LBUTTON)) {
        fps->prev_mouse_position = mp;
    }

    // Update fly camera
    if (gs_platform_mouse_down(GS_MOUSE_LBUTTON)) {
        gs_vec2 ds = gs_v2(mp.x - fps->prev_mouse_position.x, mp.y - fps->prev_mouse_position.y);
        gs_camera_offset_orientation(&fps->camera, -ds.x, -ds.y);
        gs_platform_mouse_set_position(gs_platform_main_window(), fps->prev_mouse_position.x, fps->prev_mouse_position.y);
    }

    gs_vec3 vel = {0};
    if (gs_platform_key_down(GS_KEYCODE_W)) vel = gs_vec3_add(vel, gs_camera_forward(&fps->camera));
    if (gs_platform_key_down(GS_KEYCODE_S)) vel = gs_vec3_add(vel, gs_camera_backward(&fps->camera));
    if (gs_platform_key_down(GS_KEYCODE_A)) vel = gs_vec3_add(vel, gs_camera_left(&fps->camera));
    if (gs_platform_key_down(GS_KEYCODE_D)) vel = gs_vec3_add(vel, gs_camera_right(&fps->camera));

    fps->camera.transform.position = gs_vec3_add(fps->camera.transform.position, gs_vec3_scale(gs_vec3_norm(vel), dt * CAM_SPEED));
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}






