/*================================================================
    * Copyright: 2020 John Jackson
    * uniforms_advanced

    Show how to construct multiple uniforms handles using sub-names
    (useful for struct definitions) in shaders.
    Can then upload block data for uniform binding.

    Modified from: https://learnopengl.com/Lighting/Light-casters

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

// Include all necessary vertex/shader soure data for program
#include "data.c"

static struct
{
    gs_command_buffer_t                      cb;
    gs_camera_t                              cam;
    gs_handle(gs_graphics_vertex_buffer_t)   vbo;
    gs_handle(gs_graphics_pipeline_t)        pip;
    gs_handle(gs_graphics_shader_t)          shader;
    gs_handle(gs_graphics_uniform_t)         u_model;
    gs_handle(gs_graphics_uniform_t)         u_view;
    gs_handle(gs_graphics_uniform_t)         u_proj;
    gs_handle(gs_graphics_uniform_t)         u_tex;
    gs_handle(gs_graphics_texture_t)         t_diffuse;
    gs_asset_mesh_t                          mesh;
} app;

#define ROW_COL_CT  10

// Structure to match light params definition in shader
typedef struct light_params_t {
    gs_vec3 position;
    gs_vec3 ambient;
    gs_vec3 diffuse;
    gs_vec3 specular;
    float constant;
    float linear;
    float quadratic;
} light_params_t;

void app_init()
{
    // Construct new command buffer
    app.cb = gs_command_buffer_new();

    // Set up camera
    app.cam = gs_camera_perspective();

    const char* asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
    gs_snprintfc(TMP, 256, "%s/duck/Duck.gltf", asset_dir);

    gs_asset_mesh_load_from_file(TMP, &app.mesh, &(gs_asset_mesh_decl_t){
        .layout = (gs_asset_mesh_layout_t[]){
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION}, 
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD} 
        },
        .layout_size = sizeof(gs_asset_mesh_layout_t) * 2
    }, NULL, 0);

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

    // Create diffuse texture
    app.t_diffuse = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = ROW_COL_CT,
            .height = ROW_COL_CT,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .data = pixels
        }
    ); 


    // Construct individual uniforms
    app.u_model = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_model",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    app.u_view = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_view",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    app.u_proj = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_proj",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    ); 

    app.u_tex = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_tex",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}}
        }
    ); 

    // Create shaders and pipelines
    app.shader = gs_graphics_shader_create(
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]){
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
            },
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "light_shader"
        }
    );

    app.pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = app.shader
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]) {
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3},  // Position
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}   // TexCoord
                },
                .size = 2 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    gs_vec2 fs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    const float t = gs_platform_elapsed_time() * 0.001f;
    const float rad = sin(t * 0.2f) * 10.f; 

    // Action for clearing the screen
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}}};

    // Construct proj/view matrices
    gs_mat4 view = gs_camera_get_view(&app.cam);  
    gs_mat4 proj = gs_camera_get_proj(&app.cam, (int32_t)ws.x, (int32_t)ws.y);

    // Uniform bindings that don't change per object
    gs_graphics_bind_uniform_desc_t uniforms[] = {
        {.uniform = app.u_proj, .data = &proj},
        {.uniform = app.u_view, .data = &view},
        {.uniform = app.u_tex, .data = &app.t_diffuse, .binding = 0} // Set base binding for texture samplers
    }; 

    gs_vec3 cube_positions[] = {
        {0.f, 0.f, -2.f}
    };

    gs_graphics_renderpass_begin(&app.cb, GS_GRAPHICS_RENDER_PASS_DEFAULT); {
        gs_graphics_pipeline_bind(&app.cb, app.pip);
        gs_graphics_set_viewport(&app.cb, 0, 0, (uint32_t)fs.x, (uint32_t)fs.y);
        gs_graphics_clear(&app.cb, &clear);

        for (uint32_t p = 0; p < gs_dyn_array_size(app.mesh.primitives); ++p)
        { 
            gs_asset_mesh_primitive_t* prim = &app.mesh.primitives[p];

            // Create bindings
            gs_graphics_bind_desc_t binds = {
                .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = prim->vbo}},
                .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = prim->ibo}},
                .uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
            };

            gs_graphics_apply_bindings(&app.cb, &binds);

            // For each cube
            for (uint32_t i = 0; i < sizeof(cube_positions) / sizeof(gs_vec3); ++i) 
            { 
                // Construct model matrix
                float angle = 20.0f * (i + 1);
                gs_vqs xform = (gs_vqs){
                    .translation = gs_v3(0.f, 0.f, sin(t) * 5.f),
                    .rotation = gs_quat_angle_axis(gs_deg2rad(angle), gs_v3(1.f, 0.3f, 0.5f)),
                    .scale = gs_v3s(0.5f)
                };
                gs_mat4 model = gs_vqs_to_mat4(&xform);

                gs_println("c: %zu", prim->count);

                gs_graphics_bind_desc_t model_binds = {
                    .uniforms = &(gs_graphics_bind_uniform_desc_t){.uniform = app.u_model, .data = &model}
                };
                gs_graphics_apply_bindings(&app.cb, &model_binds);
                gs_graphics_draw(&app.cb, &(gs_graphics_draw_desc_t){.start = 0, .count = prim->count});
            } 
        } 
    }
    gs_graphics_renderpass_end(&app.cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(&app.cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}






