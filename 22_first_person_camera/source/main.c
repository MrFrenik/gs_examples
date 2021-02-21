/*================================================================
    * Copyright: 2020 John Jackson
    * first person camera

    Simple first person camera example. 
    Modified from: https://www.raylib.com/examples.html
    Shows how to lock/unlock mouse.

    Controls: 
        * Mouse to rotate camera view
        * WASD to move camera

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#include "data.c"

typedef struct fps_camera_t {
    float pitch;
    float bob_time;
    gs_camera_t cam;
} fps_camera_t;

void fps_camera_update(fps_camera_t* cam);

#define rand_range(MIN, MAX)\
    (rand() % (MAX - MIN + 1) + MIN)

gs_command_buffer_t  cb  = {0};
gs_immediate_draw_t  gsi = {0};
fps_camera_t         fps = {0};

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();

    // Construct camera
    fps.cam = gs_camera_perspective();
    fps.cam.transform.position = gs_v3(4.f, 2.f, 4.f);

    // Generate columns
    for (uint32_t i = 0; i < MAX_COLUMNS; ++i) {
        heights[i] = (float)rand_range(1, 12);
        positions[i] = gs_v3((float)rand_range(-15, 15), 0.f, (float)rand_range(-15, 15));
        colors[i] = gs_color((uint8_t)rand_range(20, 255), (uint8_t)rand_range(10, 55), 30, 255);
    }

    // Lock mouse at start by default
    gs_platform_lock_mouse(gs_platform_main_window(), true);
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

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

    /* Immediate draw scene */
    gsi_camera(&gsi, &fps.cam);
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, true);

    // Draw ground plane
    gsi_box(&gsi, 0.f, -0.1f * 0.5f, 0.f, 32.f, 0.1f, 32.f, 100, 100, 100, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Draw walls
    gsi_box(&gsi, -16.0f, 2.5f, 0.0f, 1.0f, 5.0f, 32.0f, 20, 50, 220, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_box(&gsi, 16.0f, 2.5f, 0.0f, 1.0f, 5.0f, 32.0f, 20, 200, 220, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_box(&gsi, 0.0f, 2.5f, 16.0f, 32.0f, 5.0f, 1.0f, 150, 200, 20, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Draw pillars
    for (uint32_t i = 0; i < MAX_COLUMNS; ++i) {
        gs_vec3* p = &positions[i];
        gs_color_t* c = &colors[i];
        float h = heights[i];
        float hy = h * 0.5f;
        float hx = 2.f;
        float hz = 2.f;

        gsi_box(&gsi, p->x, p->y + hy, p->z, hx, h, hz  , c->r, c->g, c->b, c->a, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
        gsi_box(&gsi, p->x, p->y + hy, p->z, hx, h, hz  , 200, 50, 20, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    }

    // Draw text
    gsi_defaults(&gsi);
    gsi_camera2D(&gsi);
    gsi_rectvd(&gsi, gs_v2(10.f, 10.f), gs_v2(220.f, 70.f), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_color(10, 50, 150, 128), GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_rectvd(&gsi, gs_v2(10.f, 10.f), gs_v2(220.f, 70.f), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_color(10, 50, 220, 255), GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_text(&gsi, 20.f, 25.f, "FPS Camera Controls:", NULL, false, 0, 0, 0, 255);
    gsi_text(&gsi, 40.f, 40.f, "- Move: W, A, S, D", NULL, false, 20, 20, 20, 255);
    gsi_text(&gsi, 40.f, 55.f, "- Mouse to look", NULL, false, 20, 20, 20, 255);
    gsi_text(&gsi, 40.f, 70.f, "- Shift to run", NULL, false, 20, 20, 20, 255);

    /* Render */
    gsi_render_pass_submit(&gsi, &cb, GS_COLOR_WHITE);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

void fps_camera_update(fps_camera_t* fps)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);

    gs_vec2 dp = gs_vec2_scale(gs_platform_mouse_deltav(), SENSITIVITY);
    const float mod = gs_platform_key_down(GS_KEYCODE_LEFT_SHIFT) ? 2.f : 1.f; 
    float dt = platform->time.delta;
    float pitch = fps->pitch;

    // Keep track of previous amount to clamp the camera's orientation
    fps->pitch = gs_clamp(pitch + dp.y, -90.f, 90.f);
    dp.y = (pitch + dp.y <= 90.f && pitch + dp.y >= -90.f) ? dp.y : 0.f;

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
        fps->bob_time += dt * 8.f;
        float sb = sin(fps->bob_time);
        float bob_amt = (sb * 0.5f + 0.5f) * 0.1f * mod;
        float rot_amt = sb * 0.0004f * mod;
        fps->cam.transform.position.y = 2.f + bob_amt;        
        fps->cam.transform.rotation = gs_quat_mul(fps->cam.transform.rotation, gs_quat_angle_axis(rot_amt, GS_ZAXIS));
    }
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}   






