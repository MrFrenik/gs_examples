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

gs_command_buffer_t                      cb         = {0};
gs_camera_t                              cam        = {0};
gs_handle(gs_graphics_vertex_buffer_t)   vbo        = {0};
gs_handle(gs_graphics_pipeline_t)        pip        = {0};
gs_handle(gs_graphics_shader_t)          shader     = {0};
gs_handle(gs_graphics_texture_t)         t_diffuse  = {0};
gs_handle(gs_graphics_texture_t)         t_specular = {0};
gs_handle(gs_graphics_uniform_t)         u_model    = {0};
gs_handle(gs_graphics_uniform_t)         u_view     = {0};
gs_handle(gs_graphics_uniform_t)         u_proj     = {0};
gs_handle(gs_graphics_uniform_t)         u_viewpos  = {0};
gs_handle(gs_graphics_uniform_t)         u_light    = {0};
gs_handle(gs_graphics_uniform_t)         u_material = {0};

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

typedef struct material_params_t {
    gs_handle(gs_graphics_texture_t) diffuse;
    gs_handle(gs_graphics_texture_t) specular;
    float shininess;
} material_params_t;

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();

    // Set up camera
    cam = gs_camera_perspective();
    cam.transform.position = gs_v3(0.f, 0.f, 3.f);

    // Construct vertex buffer
    vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_vertex_buffer_desc_t) {
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

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
    t_diffuse = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = ROW_COL_CT,
            .height = ROW_COL_CT,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .data = pixels
        }
    );

    // 1x1 white texture
    gs_color_t white = GS_COLOR_WHITE;

    // Create spec texture
    t_specular = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = 1,
            .height = 1,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
            .data = &white
        }
    );

    // Construct individual uniforms
    u_model = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_model",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    u_view = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_view",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    u_proj = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_proj",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_MAT4}}
        }
    );

    u_viewpos = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_viewpos",
            .layout = (gs_graphics_uniform_layout_desc_t[]){{.type = GS_GRAPHICS_UNIFORM_VEC3}}
        }
    );

    // Method for batch constructing uniforms within structures (that way you can upload data as a unit, rather than individuals)
    u_light = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_light", 
            .layout = (gs_graphics_uniform_layout_desc_t[]){                    // Layout description must match light structure description
                {.type = GS_GRAPHICS_UNIFORM_VEC3, .fname = ".position"},
                {.type = GS_GRAPHICS_UNIFORM_VEC3, .fname = ".ambient"},
                {.type = GS_GRAPHICS_UNIFORM_VEC3, .fname = ".diffuse"},
                {.type = GS_GRAPHICS_UNIFORM_VEC3, .fname = ".specular"},
                {.type = GS_GRAPHICS_UNIFORM_FLOAT, .fname = ".constant"},
                {.type = GS_GRAPHICS_UNIFORM_FLOAT, .fname = ".linear"},
                {.type = GS_GRAPHICS_UNIFORM_FLOAT, .fname = ".quadratic"}
            },
            .layout_size = 7 * sizeof(gs_graphics_uniform_layout_desc_t)
        }
    );

    u_material = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_material", 
            .layout = (gs_graphics_uniform_layout_desc_t[]){                    // Layout description must match material structure description
                {.type = GS_GRAPHICS_UNIFORM_SAMPLER2D, .fname = ".diffuse"},
                {.type = GS_GRAPHICS_UNIFORM_SAMPLER2D, .fname = ".specular"},
                {.type = GS_GRAPHICS_UNIFORM_FLOAT, .fname = ".shininess"}
            },
            .layout_size = 3 * sizeof(gs_graphics_uniform_layout_desc_t) 
        }
    );

    // Create shaders and pipelines
    shader = gs_graphics_shader_create(
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]){
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
            },
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "light_shader"
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
            .depth = {
                .func = GS_GRAPHICS_DEPTH_FUNC_LESS
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]) {
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3},  // Position
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3},  // Normal
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2}   // TexCoord
                },
                .size = 3 * sizeof(gs_graphics_vertex_attribute_desc_t)
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
    gs_mat4 view = gs_camera_get_view(&cam);  
    gs_mat4 proj = gs_camera_get_proj(&cam, (int32_t)ws.x, (int32_t)ws.y);

    // Construct light data to submit
    light_params_t light = {
        .position = gs_v3(sin(t) * rad, 1.f, cos(t) * rad),
        .ambient = gs_v3(0.2f, 0.2f, 0.2f),
        .diffuse = gs_v3(0.5f, 0.5f, 0.5f),
        .specular = gs_v3(1.f, 1.f, 1.f),
        .constant = 1.f,
        .linear = 0.09f,
        .quadratic = 0.032f
    };

    // Construct material data to submit
    material_params_t mat = {
        .diffuse = t_diffuse,
        .specular = t_specular,
        .shininess = 32.f
    };

    // Uniform bindings that don't change per object
    gs_graphics_bind_uniform_desc_t uniforms[] = {
        {.uniform = u_proj, .data = &proj},
        {.uniform = u_view, .data = &view},
        {.uniform = u_light, .data = &light},
        {.uniform = u_viewpos, .data = &cam.transform.position},
        {.uniform = u_material, .data = &mat, .binding = 0}         // Set base binding for texture samplers
    };

    // Create bindings
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = vbo}},
        .uniforms = {.desc = uniforms, .size = sizeof(uniforms)}
    };

    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT); {
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_set_viewport(&cb, 0, 0, (uint32_t)fs.x, (uint32_t)fs.y);
        gs_graphics_clear(&cb, &clear);
        gs_graphics_apply_bindings(&cb, &binds);

        // For each cube
        for (uint32_t i = 0; i < sizeof(cube_positions) / sizeof(gs_vec3); ++i) {
            // Construct model matrix
            gs_vec3* pos = &cube_positions[i];
            gs_mat4 model = gs_mat4_translate(pos->x, pos->y, pos->z);
            float angle = 20.0f * (i + 1);
            model = gs_mat4_mul(model, gs_mat4_rotatev(gs_deg2rad(angle), gs_v3(1.0f, 0.3f, 0.5f)));

            gs_graphics_bind_desc_t model_binds = {
                .uniforms = &(gs_graphics_bind_uniform_desc_t){.uniform = u_model, .data = &model}
            };
            gs_graphics_apply_bindings(&cb, &model_binds);
            gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
        } 
    }
    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}






