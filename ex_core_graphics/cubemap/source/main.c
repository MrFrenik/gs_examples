/*================================================================
    * Copyright: 2020 John Jackson
    * cubemap

    The purpose of this example is to demonstrate how to create a 
    cubemap texture and explicitly construct GPU resources to use 
    for your application.

    NOTE: Cubemaps REQUIRE that all face images are the exact same size and 
    both width and height are the same.
    If not, the cubemap will be ill-formed and most likely 
    crash in the graphics driver.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

// All necessary graphics data for this example
#include "data.c"

typedef struct
{
    gs_command_buffer_t cb;
    gs_handle(gs_graphics_texture_t) tex;
    gs_handle(gs_graphics_pipeline_t) pip;
    gs_handle(gs_graphics_vertex_buffer_t) vbo;
    gs_handle(gs_graphics_index_buffer_t) ibo;
    gs_handle(gs_graphics_uniform_t) u_tex;
    gs_handle(gs_graphics_uniform_t) u_proj;
    gs_handle(gs_graphics_uniform_t) u_view;
    gs_handle(gs_graphics_uniform_t) u_model;
    gs_handle(gs_graphics_texture_t) tex0;
    gs_immediate_draw_t gsi;
    gs_camera_t camera;
} app_t;

void app_camera_update();

void app_init()
{
    app_t* app = gs_user_data(app_t);

    app->cb = gs_command_buffer_new(); 
    app->gsi = gs_immediate_draw_new(gs_platform_main_window()); 
    app->camera = gs_camera_perspective();

    // Load texture data for all cubemap sides 
    const char* asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
    struct {const char* path; void* data; uint32_t width; uint32_t height; uint32_t ncomps;} tdata[] = {
        {.path = "right.jpg"},
        {.path = "left.jpg"},
        {.path = "top.jpg"},
        {.path = "bottom.jpg"},
        {.path = "front.jpg"},
        {.path = "back.jpg"}
    };

    for (uint32_t i = 0; i < 6; ++i) {
        gs_snprintfc(PATH, 256, "%s/%s", asset_dir, tdata[i].path);
        gs_util_load_texture_data_from_file(PATH, &tdata[i].width, &tdata[i].height, &tdata[i].ncomps, &tdata[i].data, false);
        gs_assert(tdata[i].data);
        gs_println("%zu", tdata[i].ncomps);
    } 

    // Want to be able to give a single texture with offsets for where to read the data...

    // Construct cubemap texture and upload all required data for it
	gs_graphics_texture_desc_t cdesc = (gs_graphics_texture_desc_t){
        .type = GS_GRAPHICS_TEXTURE_CUBEMAP,
        .width = tdata[0].width,
        .height = tdata[0].height,
        .format = tdata[0].ncomps == 3 ? GS_GRAPHICS_TEXTURE_FORMAT_RGB8 : GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
        .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
        .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
        .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
        .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
		.wrap_r = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE
	};
	for (uint32_t i = 0; i < 6; ++i) { 
		cdesc.data[i] = tdata[i].data;
	}
	app->tex = gs_graphics_texture_create(&cdesc); 

    app->tex0 = gs_graphics_texture_create(&(gs_graphics_texture_desc_t){
        .type = GS_GRAPHICS_TEXTURE_2D,
        .width = tdata[0].width,
        .height = tdata[0].height,
        .data = tdata[0].data,
        .format = tdata[0].ncomps == 3 ? GS_GRAPHICS_TEXTURE_FORMAT_RGB8 : GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
        .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
        .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
        .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE,
        .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE
    });

    // Skybox vbo/ibo
    app->vbo = gs_graphics_vertex_buffer_create(&(gs_graphics_vertex_buffer_desc_t){
        .data = v_data,
        .size = sizeof(v_data)
    });

    app->ibo = gs_graphics_index_buffer_create(&(gs_graphics_index_buffer_desc_t){
        .data = i_data, 
        .size = sizeof(i_data)
    });

    // Create shader
    gs_handle(gs_graphics_shader_t) shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = (gs_graphics_shader_source_desc_t[]){
                {.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
                {.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
            }, 
            .size = 2 * sizeof(gs_graphics_shader_source_desc_t),
            .name = "skybox"
        }
    );

    // Construct uniforms
    app->u_tex = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "u_tex",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLERCUBE}
        }
    );

    app->u_proj = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "u_proj",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    app->u_view = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "u_view",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    app->u_model = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
            .name = "u_model",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    // Skybox pipeline
    app->pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .primitive = GS_GRAPHICS_PRIMITIVE_QUADS,
                .shader = shader,
                .index_buffer_element_size = sizeof(uint16_t)
            },
            .depth = {
                .func = GS_GRAPHICS_DEPTH_FUNC_ALWAYS
            },
            .layout = {
                .attrs = (gs_graphics_vertex_attribute_desc_t[]){
                    {.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3, .name = "a_pos"}
                },
                .size = 1 * sizeof(gs_graphics_vertex_attribute_desc_t)
            }
        }
    );
}

void app_update()
{ 
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_t* cb = &app->cb;
    gs_immediate_draw_t* gsi = &app->gsi;
    const float t = gs_platform_time()->elapsed;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit(); 

    // Update fly cam
    if (gs_platform_mouse_down(GS_MOUSE_RBUTTON)) {
        gs_platform_lock_mouse(gs_platform_main_window(), true);
        app_camera_update();
    }
    else {
        gs_platform_lock_mouse(gs_platform_main_window(), false);
    } 

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t){.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};

    gs_mat4 model = gs_mat4_scalev(gs_v3s(1000.f));
    gs_mat4 view = gs_camera_get_view(&app->camera);
    gs_mat4 proj = gs_camera_get_proj(&app->camera, (uint32_t)fbs.x, (uint32_t)fbs.y);

    // Bindings for all buffers: vertex, index, sampler
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = app->vbo}},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = app->ibo}},
        .uniforms = {
            .desc = (gs_graphics_bind_uniform_desc_t[]){
                {.uniform = app->u_tex, .data = &app->tex, .binding = 0},
                {.uniform = app->u_proj, .data = &proj},
                {.uniform = app->u_view, .data = &view},
                {.uniform = app->u_model, .data = &model}
            },
            .size = 4 * sizeof(gs_graphics_bind_uniform_desc_t)
        }
    };

    /* Render */

    gsi_camera(gsi, &app->camera, (uint32_t)fbs.x, (uint32_t)fbs.y);
    gsi_box(gsi, 0.f, 0.f, -1.f, 0.5f, 0.5f, 0.5f, 0, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_defaults(gsi);
    gsi_camera2D(gsi, (uint32_t)fbs.x, (uint32_t)fbs.y);
    gsi_text(gsi, 10, 10, "Holding Right MButton to look, while holding use WASD to move", NULL, false, 0, 0, 0, 255);

    gs_graphics_renderpass_begin(cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(cb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(cb, &clear);
        gs_graphics_pipeline_bind(cb, app->pip); 
        gs_graphics_apply_bindings(cb, &binds);
        gs_graphics_draw(cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
    gs_graphics_renderpass_end(cb);

    gsi_renderpass_submit_ex(&app->gsi, cb, (uint32_t)fbs.x, (uint32_t)fbs.y, NULL);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_free(&app->cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown
    };
}   

#define SENSITIVITY 0.2f
static float pitch = 0.f;
static float speed = 2.f;
void app_camera_update()
{ 
    app_t* app = gs_user_data(app_t);
    gs_platform_t* platform = gs_subsystem(platform);
    gs_vec2 dp = gs_vec2_scale(gs_platform_mouse_deltav(), SENSITIVITY);
    const float mod = gs_platform_key_down(GS_KEYCODE_LEFT_SHIFT) ? 2.f : 1.f; 
    float dt = platform->time.delta;
    float old_pitch = pitch;
    gs_camera_t* camera = &app->camera; 

    // Keep track of previous amount to clamp the camera's orientation
    pitch = gs_clamp(old_pitch + dp.y, -90.f, 90.f);

    // Rotate camera
    gs_camera_offset_orientation(camera, -dp.x, old_pitch - pitch);

    gs_vec3 vel = {0};
    switch (camera->proj_type)
    {
        case GS_PROJECTION_TYPE_ORTHOGRAPHIC:
        {
            if (gs_platform_key_down(GS_KEYCODE_W)) vel = gs_vec3_add(vel, gs_camera_up(camera));
            if (gs_platform_key_down(GS_KEYCODE_S)) vel = gs_vec3_add(vel, gs_vec3_scale(gs_camera_up(camera), -1.f));
            if (gs_platform_key_down(GS_KEYCODE_A)) vel = gs_vec3_add(vel, gs_camera_left(camera));
            if (gs_platform_key_down(GS_KEYCODE_D)) vel = gs_vec3_add(vel, gs_camera_right(camera)); 

            // Ortho scale
            gs_vec2 wheel = gs_platform_mouse_wheelv();
            camera->ortho_scale -= wheel.y;
        } break;

        case  GS_PROJECTION_TYPE_PERSPECTIVE:
        {
            if (gs_platform_key_down(GS_KEYCODE_W)) vel = gs_vec3_add(vel, gs_camera_forward(camera));
            if (gs_platform_key_down(GS_KEYCODE_S)) vel = gs_vec3_add(vel, gs_camera_backward(camera));
            if (gs_platform_key_down(GS_KEYCODE_A)) vel = gs_vec3_add(vel, gs_camera_left(camera));
            if (gs_platform_key_down(GS_KEYCODE_D)) vel = gs_vec3_add(vel, gs_camera_right(camera)); 
            gs_vec2 wheel = gs_platform_mouse_wheelv();
            speed = gs_clamp(speed + wheel.y, 0.01f, 50.f);
        } break;
    }

    camera->transform.position = gs_vec3_add(camera->transform.position, gs_vec3_scale(gs_vec3_norm(vel), dt * speed * mod));
} 






