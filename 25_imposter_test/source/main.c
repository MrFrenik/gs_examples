/*================================================================
    * Copyright: 2020 John Jackson
    * HelloGS

    A Bare bones application for getting started using `gunslinger`.
    Creates an appplication context, an engine context, and then 
    opens a main window for you using the rendering context.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

// Load up the duck mesh then draw it using this method (see what it looks like)
gs_asset_mesh_t duck                      = {0};
gs_asset_texture_t tex                    = {0};
gs_handle(gs_graphics_render_pass_t) rp   = {0};
gs_handle(gs_graphics_framebuffer_t) fbo  = {0};
gs_handle(gs_graphics_texture_t)     rt   = {0};
gs_dyn_array(gs_vec3) positions = NULL;

typedef struct fps_camera_t {
    float pitch;
    float bob_time;
    gs_camera_t cam;
} fps_camera_t;

// Camera defines
#define CAM_SPEED   5
#define SENSITIVITY  0.2f

void fps_camera_update(fps_camera_t* cam);

gs_immediate_draw_t gsi = {0};
gs_command_buffer_t cb = {0};
fps_camera_t        fps = {0};

void app_init()
{
    gsi = gs_immediate_draw_new();
    cb  = gs_command_buffer_new();

    // Construct camera
    fps.cam = gs_camera_perspective();
    fps.cam.transform.position = gs_v3(0.f, 2.f, 1.f);
    // fps.cam.fov = 20.f;

    // Lock mouse at start by default
    gs_platform_lock_mouse(gs_platform_main_window(), true);

    gs_asset_mesh_layout_t mesh_layout[] = {
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION},  // Float3
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD},  // Float2
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR}      // Byte4
    };

    gs_asset_mesh_decl_t mesh_decl = {
        .layout = mesh_layout,
        .layout_size = sizeof(mesh_layout)
    };

    // Load asset data
    gs_asset_mesh_load_from_file("./assets/duck/Duck.gltf", &duck, &mesh_decl, NULL, 0);
    gs_asset_texture_load_from_file("./assets/duck/DuckCM.png", &tex, NULL, false, false);

    // Construct frame buffer
    fbo = gs_graphics_framebuffer_create(NULL);

    // Construct color render target
    rt = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t) {
            .width = gs_platform_window_width(0),   // Width of texture in pixels
            .height = gs_platform_window_height(0), // Height of texture in pixels
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,                          // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
            .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,                           // Wrapping type for s axis of texture
            .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,                           // Wrapping type for t axis of texture
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,                     // Minification filter for texture
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,                     // Magnification filter for texture
            .render_target = true
        }
    );

    // Construct render pass for offscreen render pass
    rp = gs_graphics_render_pass_create(
        &(gs_graphics_render_pass_desc_t){
            .fbo = fbo,                      // Frame buffer to bind for render pass
            .color = &rt,                    // Color buffer array to bind to frame buffer    
            .color_size = sizeof(rt)         // Size of color attachment array in bytes
        }
    );

#define rand_range(MIN, MAX)\
    (rand() % (MAX - MIN + 1) + MIN)
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    const gs_vec2 ws = gs_platform_window_sizev(0);
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(0);

    // If click, then lock again (in case lost)
    if (gs_platform_mouse_pressed(GS_MOUSE_LBUTTON) && !gs_platform_mouse_locked()) {
        fps.cam.transform.rotation = gs_quat_default();
        fps.pitch = 0.f;
        gs_platform_lock_mouse(gs_platform_main_window(), true);
    }

    // Update camera
    if (gs_platform_mouse_locked()) {
        fps_camera_update(&fps);
    }

    // Draw simple cube with forward vector
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, false);

    // Draw cube at origin
    float ydt = 0.f, xdt = 0.f;
    gsi_push_matrix(&gsi, GSI_MATRIX_PROJECTION);
    {
        // XZ rotations
        const float angle_span = 360.f / 16.f;
        const gs_vec3 pos = gs_v3(0.f, 0.f, 0.f);
        const gs_vec3 cf3 = gs_vec3_sub(pos, gs_vec3_add(fps.cam.transform.position, gs_vec3_scale(gs_camera_forward(&fps.cam), 0.1f)));

        // ydt
        {
            const gs_vec2 fc = gs_vec2_norm(gs_v2(cf3.x, cf3.z));
            const gs_vec2 fo = gs_v2(0.f, 1.f);
            const float dot = gs_vec2_dot(fc, fo); 
            const float det = fc.x * fo.y - fc.y * fo.x;
            float theta = fmodf(gs_rad2deg(atan2(det, dot)) + 180.f, 360.f);
            // gs_println("dot: %.2f, det: %.2f, theta: %.2f", dot, det, theta);
            const float tclamp = floor(theta / angle_span) * angle_span;
            ydt = (theta - tclamp);
        }

        // xdt
        {
            const float xas = 360.f / 8.f;
            const gs_vec2 fc = gs_vec2_norm(gs_v2(cf3.z, cf3.y));
            const gs_vec2 fo = gs_v2(1.f, 0.f);
            const float dot = gs_vec2_dot(fc, fo); 
            const float det = fc.x * fo.y - fc.y * fo.x;
            float theta = gs_rad2deg(atan2(det, dot));
            gs_println("dot: %.2f, det: %.2f, theta: %.2f", dot, det, theta);
            const float tclamp = floor(theta / xas) * xas;
            xdt = (theta - tclamp);
            if (dot < 0.f) xdt *= -1.f;
        }

        gs_camera_t cam = fps.cam;

        // Camera vp
        gs_mat4 pro = gs_camera_get_proj(&cam, (int32_t)ws.x, (int32_t)ws.y);
        gs_vec3 target = gs_vec3_add(cam.transform.position, gs_camera_forward(&cam));
        gs_mat4 la = gs_mat4_look_at(cam.transform.position, target, gs_camera_up(&cam));
        gs_mat4 vp = gs_mat4_mul(pro, la);

        // gsi_camera(&gsi, &cam);
        gsi_mul_matrix(&gsi, vp);
        gsi_push_matrix(&gsi,GSI_MATRIX_MODELVIEW);
        {
            // gsi_transf(&gsi, diff.x, 0.f, diff.z);
            // gsi_rotatefv(&gsi, -gs_deg2rad(xdt + 360.f), GS_XAXIS);
            gsi_rotatefv(&gsi, gs_deg2rad(ydt - 360.f), GS_YAXIS);
            gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES);
            gsi_line3Dv(&gsi, gs_v3s(0.f), GS_ZAXIS, GS_COLOR_BLUE);
        }
        gsi_pop_matrix(&gsi);
    }
    gsi_pop_matrix(&gsi);

    gs_graphics_clear_desc_t fb_clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.0f, 0.0f, 0.0f, 0.f}};

    // Bind render pass for offscreen rendering then draw to buffer
    gs_graphics_begin_render_pass(&cb, rp);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)ws.x, (int32_t)ws.y);
        gs_graphics_clear(&cb, &fb_clear);
        gsi_draw(&gsi, &cb);
        // gsi_begin(&gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

        // Get pipeline for mesh from immediate draw backend
        gsi_pipeline_state_attr_t state = {
            .depth_enabled = 1,
            .stencil_enabled = 0,
            .blend_enabled = 1,
            .face_cull_enabled = 1,
            .prim_type = (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES
        };
        gs_handle(gs_graphics_pipeline_t) pip = gsi_get_pipeline(&gsi, state);

        gs_graphics_bind_pipeline(&cb, pip); // Bind pipeline

        gs_mat4 final_rot = gs_mat4_mul_list(
            1,
            gs_mat4_rotatev(gs_deg2rad(ydt - 360.f), GS_YAXIS) 
        );

        gs_mat4 mvp = gs_camera_get_view_projection(&fps.cam, 360, 360);
        mvp = gs_mat4_mul_list(
            4, 
            mvp,
            gs_mat4_translate(2.f, 0.f, -2.f),
            final_rot,
            gs_mat4_scale(0.01f, 0.01f, 0.01f)
        );

        // For each primitive in mesh
        for (uint32_t i = 0; i < gs_dyn_array_size(duck.primitives); ++i)
        {
            gs_asset_mesh_primitive_t* prim = &duck.primitives[i];

            // Bindings for all buffers: vertex, index, uniform, sampler
            gs_graphics_bind_desc_t binds = {
                .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = prim->vbo}},
                .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = prim->ibo}},
                .uniforms = {
                    .desc = (gs_graphics_bind_uniform_desc_t[]){
                        {.uniform = gsi.uniform, .data = &mvp},
                        {.uniform = gsi.sampler, .data = &tex, .binding = 0}
                    },
                    .size = 2 * sizeof(gs_graphics_bind_uniform_desc_t)
                }
            };

            gs_graphics_apply_bindings(&cb, &binds);
            gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = prim->count});
        }

    gs_graphics_end_render_pass(&cb);

    gsi_push_matrix(&gsi, GSI_MATRIX_PROJECTION);
    {
        gsi_camera(&gsi, &fps.cam);
        gsi_push_matrix(&gsi,GSI_MATRIX_MODELVIEW);
        {
            gsi_transf(&gsi, 0.f, 0.f, 0.f);
            // gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 150, GS_GRAPHICS_PRIMITIVE_LINES);
            // gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_ZAXIS, GS_COLOR_BLUE);
        }
        gsi_pop_matrix(&gsi);
    }
    gsi_pop_matrix(&gsi);

    // Billboarded textured quad with render target
    gsi_push_matrix(&gsi, GSI_MATRIX_PROJECTION);
    {
        const float angle_span = 360.f / 16.f; 
        const gs_vec3 pos = gs_v3(0.f, 0.f, 0.f);
        const gs_vec3 cf3 = gs_vec3_sub(pos, fps.cam.transform.position);
        const float len = gs_vec3_len(cf3);
        const gs_vec2 fc = gs_vec2_norm(gs_v2(cf3.x, cf3.z));
        const gs_vec2 fo = gs_v2(0.f, 1.f);
        const float dot = gs_vec2_dot(fc, fo); 
        const float det = fc.x * fo.y - fc.y * fo.x;
        float theta = fmodf(gs_rad2deg(atan2(det, dot)) + 180.f, 360.f);
        // gs_println("dot: %.2f, det: %.2f, theta: %.2f, len: %.2f", dot, det, theta, len);
        const float tclamp = floor(theta / angle_span) * angle_span;

        // Need to rotate camera to new y rot vector
        // gs_quat rot = gs_quat_mul(fps.cam.transform.rotation, gs_quat_angle_axis(gs_deg2rad(tclamp - theta), GS_YAXIS));

        // gs_vec3 up = gs_camera_up(cam);
        // gs_vec3 forward = 
        // gs_vec3 forward = gs_camera_forward(cam);
        // gs_vec3 target = gs_vec3_add(forward, cam->transform.position);
        // return gs_mat4_look_at(cam->transform.position, target, up);

        // return (gs_quat_rotate(cam->transform.rotation, gs_v3(0.0f, 1.0f, 0.0f)));

        // Based on angle, lock y rotation to nearest rotational value
        gs_camera_t cam = fps.cam;
        // gs_vec3 euler = gs_quat_to_euler(&cam.transform.rotation);
        // const float dist = gs_vec3_len(gs_vec3_sub(cam.transform.position, gs_v3(2.f, 0.f, -2.f)));
        // gs_println("dist: %.2f", dist * dist);
        // // gs_quat xrot = gs_quat_angle_axis(-euler.x, GS_YAXIS);
        // gs_quat yrot = gs_quat_angle_axis(gs_deg2rad(theta), GS_YAXIS);
        // // cam.transform.rotation = gs_quat_mul(xrot, yrot);
        // cam.transform.rotation = rot;
        // // cam.proj_type = GS_PROJECTION_TYPE_ORTHOGRAPHIC;
        // cam.ortho_scale = dist;
        // gs_mat4 vp = gs_camera_get_view_projection(&cam, (int32_t)ws.x, (int32_t)ws.y);

        // Need to rotate camera to new y rot vector
        gs_quat rot = gs_quat_mul(fps.cam.transform.rotation, gs_quat_angle_axis(gs_deg2rad(tclamp - theta), GS_YAXIS));
        // Based on rotation, move position of camera? That'll change scale though.
        // cam.transform.rotation = rot;
        cam.transform.position.x = cos(gs_deg2rad(tclamp)) * len;
        cam.transform.position.z = sin(gs_deg2rad(tclamp)) * len;

        // Move camera into position and orient correctly

        // gs_mat4_look_at(gs_vec3 position, gs_vec3 target, gs_vec3 up)
        // cam.transform.position = gs_v3(0.f, 2.f, -2.f);
        // Have to move the camera's position and change rotation for this to work properly
        gs_mat4 pro = gs_camera_get_proj(&cam, (int32_t)ws.x, (int32_t)ws.y);
        gs_vec3 target = gs_vec3_add(cam.transform.position, gs_camera_forward(&cam));
        // target = pos;
        gs_mat4 la = gs_mat4_look_at(cam.transform.position, target, gs_camera_up(&cam));
        gs_mat4 vp = gs_mat4_mul(pro, la);

        // This isn't working correctly, but the rotation snapping is correct. Just need to position the object in the correctly now.
        // gsi_mul_matrix(&gsi, vp);
        // // // gsi_camera(&gsi, &fps.cam);
        // gsi_push_matrix(&gsi,GSI_MATRIX_MODELVIEW);
        // {
        //     gs_vec3 trans = gs_v3(cam.transform.position.x, 0.f, cam.transform.position.z);
        //     gsi_transf(&gsi, trans.x, trans.y, trans.z);
        //     // gsi_rotatefv(&gsi, -gs_deg2rad(360.f - tclamp - theta), GS_YAXIS);
        //     gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
        //     gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_ZAXIS, GS_COLOR_BLUE);
        // }
        // gsi_pop_matrix(&gsi);

        // Determine bounds of object

        // gsi_camera(&gsi, &fps.cam);
        gsi_camera2D(&gsi);
        gsi_texture(&gsi, rt);
        gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
        {
            // Have to place the full screen quad onto quad correctly (probably with UVs to determine screen bounds of object)
            // gsi_transf(&gsi, pos.x, pos.y, pos.z);
            // gsi_scalef(&gsi, 10.f, 10.f, 10.f);
            gsi_rectvd(&gsi, gs_v2(0.f, 0.f), ws, gs_v2(0.f, 1.f), gs_v2(1.f, 0.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
        }
        gsi_pop_matrix(&gsi);
    }
    gsi_pop_matrix(&gsi);

    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}   

void fps_camera_update(fps_camera_t* fps)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);

    gs_vec2 dp = gs_vec2_scale(gs_platform_mouse_deltav(), SENSITIVITY);
    const float mod = gs_platform_key_down(GS_KEYCODE_LEFT_SHIFT) ? 5.f : 1.f; 
    float dt = platform->time.delta;
    float pitch = fps->pitch;

    // Keep track of previous amount to clamp the camera's orientation
    dp.y = (pitch + dp.y <= 90.f && pitch + dp.y >= -90.f) ? dp.y : 0.f;
    fps->pitch = gs_clamp(pitch + dp.y, -90.f, 90.f);

    // Rotate camera
    gs_camera_offset_orientation(&fps->cam, -dp.x, -dp.y);

    gs_vec3 vel = {0};
    if (gs_platform_key_down(GS_KEYCODE_W)) vel = gs_vec3_add(vel, gs_camera_forward(&fps->cam));
    if (gs_platform_key_down(GS_KEYCODE_S)) vel = gs_vec3_add(vel, gs_camera_backward(&fps->cam));
    if (gs_platform_key_down(GS_KEYCODE_A)) vel = gs_vec3_add(vel, gs_camera_left(&fps->cam));
    if (gs_platform_key_down(GS_KEYCODE_D)) vel = gs_vec3_add(vel, gs_camera_right(&fps->cam));

    // For a non-flying first person camera, need to lock the y movement velocity
    vel.y = 0.f;

    fps->cam.transform.position = gs_vec3_add(fps->cam.transform.position, gs_vec3_scale(gs_vec3_norm(vel), dt * CAM_SPEED * mod));

    // If moved, then we'll "bob" the camera some
    if (gs_vec3_len(vel) != 0.f) {
        // fps->bob_time += dt * 8.f;
        // float sb = sin(fps->bob_time);
        // float bob_amt = (sb * 0.5f + 0.5f) * 0.1f * mod;
        // float rot_amt = sb * 0.0004f * mod;
        // fps->cam.transform.position.y = 2.f + bob_amt;        
        // fps->cam.transform.rotation = gs_quat_mul(fps->cam.transform.rotation, gs_quat_angle_axis(rot_amt, GS_ZAXIS));
    }
}
