/*================================================================
    * Copyright: 2020 John Jackson
    * Behavior Tree

    Simple Behavior Tree example.  

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

#define GS_AI_IMPL
#include <gs/util/gs_ai.h> 

enum 
{
    AI_STATE_MOVE = 0x00,
    AI_STATE_HEAL
};

typedef struct
{
    gs_vqs xform;
    gs_vec3 target;
    float health;
    int16_t state;
} ai_t;

typedef struct
{
    gs_command_buffer_t cb;
    gs_gui_context_t gui;
    gs_immediate_draw_t gsi;
    gs_camera_t camera;
    gs_ai_bt_t bt;
    ai_t ai;
} app_t;

void app_camera_update();

// Behavior tree functions
void ai_behavior_tree_frame(struct gs_ai_bt_t* ctx); 
void ai_task_target_find(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node);
void ai_task_target_move_to(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node);
void ai_task_health_check(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node);
void ai_task_heal(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node);

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new();
    app->gui = gs_gui_new(gs_platform_main_window());
    app->gsi = gs_immediate_draw_new(gs_platform_main_window());

    // Set up camera
    app->camera = gs_camera_perspective();
    app->camera.transform = (gs_vqs){
        .translation = gs_v3(22.52f, 15.52f, 18.18f),
        .rotation = (gs_quat){-0.23f, 0.42f, 0.11f, 0.87f}, 
        .scale = gs_v3s(1.f)
    };

    // Initialize ai information
    app->ai = (ai_t) {
        .xform = (gs_vqs) {
            .translation = gs_v3s(0.f), 
            .rotation = gs_quat_default(), 
            .scale = gs_v3s(1.f)
        },
        .target = gs_v3s(0.f),
        .health = 100.f
    };

    // Behavior tree contexts hold onto a to an internal ai context. 
    // This context can hold global/shared information to which the nodes of the BT can read/write.
    // We'll set our BT's internal context user data to the address of our AI's information.
    app->bt.ctx.user_data = &app->ai;
}

void app_update()
{
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_t* cb = &app->cb;
    gs_immediate_draw_t* gsi = &app->gsi;
    gs_gui_context_t* gui = &app->gui;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit(); 

    if (gs_platform_mouse_down(GS_MOUSE_RBUTTON)) {
        gs_platform_lock_mouse(gs_platform_main_window(), true);
        app_camera_update();
    }
    else {
        gs_platform_lock_mouse(gs_platform_main_window(), false);
    }

    ai_behavior_tree_frame(&app->bt);

    // Update/render scene
    gsi_camera(gsi, &app->camera, (uint32_t)fbs.x, (uint32_t)fbs.y);
    gsi_depth_enabled(gsi, true);

    // Render ground
    gsi_rect3Dv(gsi, gs_v3(-15.f, -0.5f, -15.f), gs_v3(15.f, -0.5f, 15.f), gs_v2s(0.f), gs_v2s(1.f), gs_color(50, 50, 50, 255), GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Render ai 
    gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);
        gsi_mul_matrix(gsi, gs_vqs_to_mat4(&app->ai.xform));
        gsi_box(gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_pop_matrix(gsi);

    // Render target
    gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);
        gsi_translatev(gsi, app->ai.target);
        gsi_box(gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_pop_matrix(gsi);

    // Submit immediate draw
    gsi_renderpass_submit(gsi, cb, gs_color(10, 10, 10, 255));

    // Do gui
    gs_gui_begin(gui, fbs);
    { 
        gs_gui_window_begin(gui, "AI", gs_gui_rect(10, 10, 250, 200));
        gs_gui_layout_row(gui, 1, (int[]){-1}, 78);
        gs_gui_text(gui, "The AI will continue to move towards a random location as long as its health is not lower than 50." 
            "If health drops below 50, the AI will pause to heal back up to 100 then continue moving towards its target."
            "After it reaches its target, it will find another random location to move towards.");

        gs_gui_layout_row(gui, 1, (int[]){-1}, 0);
        gs_gui_label(gui, "state: %s", app->ai.state == AI_STATE_HEAL ? "HEAL" : "MOVE");
        gs_gui_layout_row(gui, 2, (int[]){55, 50}, 0);
        gs_gui_label(gui, "health: ");
        gs_gui_number(gui, &app->ai.health, 0.1f);
        gs_gui_window_end(gui);
    }
    gs_gui_end(gui);

    gs_gui_renderpass_submit_ex(gui, cb, NULL);
    gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_free(&app->cb);
    gs_immediate_draw_free(&app->gsi);
    gs_gui_free(&app->gui);
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

void ai_behavior_tree_frame(struct gs_ai_bt_t* ctx)
{
    ai_t* ai = (ai_t*)ctx->ctx.user_data;

    gsai_bt(ctx, {
        gsai_repeater(ctx, {
            gsai_selector(ctx, {

                // Heal
                gsai_sequence(ctx, { 
                    gsai_leaf(ctx, ai_task_health_check);
                    gsai_leaf(ctx, ai_task_heal);
                });

                // Move to
                gsai_sequence(ctx, {
                    gsai_leaf(ctx, ai_task_target_find);
                    gsai_condition(ctx, (ai->health > 50.f), {
                        gsai_leaf(ctx, ai_task_target_move_to);
                    });
                }); 

            });
        });
    }); 
} 

void ai_task_health_check(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node)
{
    ai_t* ai = (ai_t*)ctx->ctx.user_data;
    node->state = ai->health >= 50 ? GS_AI_BT_STATE_FAILURE : GS_AI_BT_STATE_SUCCESS;
}

void ai_task_heal(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node)
{
    ai_t* ai = (ai_t*)ctx->ctx.user_data;
    if (ai->health < 100.f) {
        ai->health += 1.f;
        node->state = GS_AI_BT_STATE_RUNNING; 
        ai->state = AI_STATE_HEAL;
    }
    else {
        node->state = GS_AI_BT_STATE_SUCCESS;
    }
}

void ai_task_target_find(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node)
{
    ai_t* ai = (ai_t*)ctx->ctx.user_data;
    float dist = gs_vec3_dist(ai->xform.translation, ai->target);
    if (dist < 1.f) {
        gs_mt_rand_t rand = gs_rand_seed(time(NULL));
        gs_vec3 target = gs_v3(
            gs_rand_gen_range(&rand, -10.f, 10.f),
            0.f,
            gs_rand_gen_range(&rand, -10.f, 10.f)
        );
        ai->target = target;
    }
    node->state = GS_AI_BT_STATE_SUCCESS;
} 

void ai_task_target_move_to(struct gs_ai_bt_t* ctx, struct gs_ai_bt_node_t* node)
{
    ai_t* ai = (ai_t*)ctx->ctx.user_data;
    const float dt = gs_platform_time()->delta;
    float dist = gs_vec3_dist(ai->xform.translation, ai->target);
    float speed = 25.f * dt;
    if (dist > speed) {
        gs_vec3 vel = gs_vec3_scale(gs_vec3_norm(gs_vec3_sub(ai->target, ai->xform.translation)), speed);
        gs_vec3 np = gs_vec3_add(ai->xform.translation, vel);
        ai->xform.translation = gs_v3(
            gs_interp_linear(ai->xform.translation.x, np.x, 0.05f),
            gs_interp_linear(ai->xform.translation.y, np.y, 0.05f),
            gs_interp_linear(ai->xform.translation.z, np.z, 0.05f)
        );
        node->state = GS_AI_BT_STATE_RUNNING;
        ai->state = AI_STATE_MOVE;
    }
    else {
        node->state = GS_AI_BT_STATE_SUCCESS;
    }
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

















