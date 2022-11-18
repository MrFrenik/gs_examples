/*================================================================
    * Copyright: 2020 John Jackson
    * flecs
    
    Simple application to hook up flecs to gunslinger.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#include <flecs/flecs.h>

#define MAX_ENTITY_COUNT    100

typedef gs_vec2 position_t;     // Position component
typedef gs_vec2 velocity_t;     // Velocity component
typedef gs_vec2 bounds_t;       // Bounds component
typedef gs_color_t color_t;     // Color component

typedef struct app_data_t {
    gs_command_buffer_t gcb;
    gs_immediate_draw_t gsi;
    ecs_world_t* world;
    gs_dyn_array(ecs_entity_t) entities;
} app_data_t;

int32_t random_val(int32_t lower, int32_t upper)
{ 
    return ((rand() % (upper - lower + 1)) + lower);
} 

// Move system
void move_system(ecs_iter_t* it) 
{
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Get columns from system signature and cache local pointers to arrays
    ECS_COLUMN(it, position_t, p, 1);
    ECS_COLUMN(it, velocity_t, v, 2);
    ECS_COLUMN(it, bounds_t, b, 3);

    for (int32_t i = 0; i < it->count; ++i) 
    {
        gs_vec2 min = gs_vec2_add(p[i], v[i]);
        gs_vec2 max = gs_vec2_add(min, b[i]);

        // Resolve collision and change velocity direction if necessary
        if (min.x < 0 || max.x >= ws.x) {
            v[i].x *= -1.f;
        }
        if (min.y < 0 || max.y >= ws.y) {
            v[i].y *= -1.f;
        }
        p[i] = gs_vec2_add(p[i], v[i]);
    }
}

void render_system(ecs_iter_t* it)
{
    app_data_t* app = gs_user_data(app_data_t);
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    // Grab position from column data
    ECS_COLUMN(it, position_t, p, 1);
    ECS_COLUMN(it, bounds_t, b, 2);
    ECS_COLUMN(it, color_t, c, 3);

    gsi_camera2D(&app->gsi, fbs.x, fbs.y);

    // Render all into immediate draw instance data
    for (int32_t i = 0; i < it->count; ++i) {
        gsi_rectvd(&app->gsi, p[i], b[i], gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), c[i], GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
}

void app_init()
{
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());
    app_data_t* app = gs_user_data(app_data_t);

    // Create command buffer and immediate mode graphics instance
    app->gcb = gs_command_buffer_new();
    app->gsi = gs_immediate_draw_new(gs_platform_main_window());

    // Create world
    app->world = ecs_init_w_args(0, NULL);

    // Register component with world
    ECS_COMPONENT(app->world, position_t);
    ECS_COMPONENT(app->world, velocity_t);
    ECS_COMPONENT(app->world, bounds_t);
    ECS_COMPONENT(app->world, color_t);

    // Register system with world
    ECS_SYSTEM(app->world, move_system, EcsOnUpdate, position_t, velocity_t, bounds_t);
    ECS_SYSTEM(app->world, render_system, EcsOnUpdate, position_t, bounds_t, color_t);

    // Create entities with random data
    for (uint32_t i = 0; i < MAX_ENTITY_COUNT; ++i)
    {
        ecs_entity_t e = ecs_new_w_type(app->world, 0);

        gs_vec2 bounds = gs_v2((f32)random_val(10, 100), (f32)random_val(10, 100)); 

        // Set data for entity
        ecs_set(app->world, e, position_t, {(f32)random_val(0, (int32_t)ws.x - (int32_t)bounds.x), (f32)random_val(0, (int32_t)ws.y - (int32_t)bounds.y)});
        ecs_set(app->world, e, velocity_t, {(f32)random_val(1, 10), (f32)random_val(1, 10)});
        ecs_set(app->world, e, bounds_t, {(f32)random_val(10, 100), (f32)random_val(10, 100)});
        ecs_set(app->world, e, color_t, {random_val(50, 255), random_val(50, 255), random_val(50, 255), 255});
    }
}

void app_update()
{
   if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

   app_data_t* app = gs_user_data(app_data_t);
   ecs_world_t* world = app->world;
   gs_command_buffer_t* gcb = &app->gcb;
   gs_immediate_draw_t* gsi = &app->gsi;
   const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

   // Progress ecs world
   ecs_progress(world, 0);

   // Render scene
   gs_snprintfc(tmp, 256, "FPS: %.2f", gs_subsystem(platform)->time.frame);
   gsi_camera2D(gsi, fbs.x, fbs.y);
   gsi_rectvd(gsi, gs_v2(0.f, 0.f), gs_v2(100.f, 20.f), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), GS_COLOR_BLACK, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
   gsi_text(gsi, 10.f, 15.f, tmp, NULL, false, 255, 255, 255, 255);

   gsi_renderpass_submit(gsi, gcb, gs_v4(0.f, 0.f, fbs.x, fbs.y), gs_color(10, 10, 10, 255));
   gs_graphics_command_buffer_submit(gcb);
}

void app_shutdown()
{
    app_data_t* app = gs_user_data(app_data_t);

    // Free all application resources
    ecs_fini(app->world);

    // Free app
    gs_free(app);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .update = app_update,
        .init = app_init,
        .shutdown = app_shutdown,
        .user_data = gs_malloc_init(app_data_t)
    };
}   
