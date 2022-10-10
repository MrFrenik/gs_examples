/*================================================================
    * Copyright: 2020 John Jackson
    * simple collision detection example

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

#define GS_PHYSICS_IMPL
#include <gs/util/gs_physics.h>

#include "data.c" 

typedef struct 
{
    gs_command_buffer_t cb;
    gs_immediate_draw_t gsi;
    gs_gui_context_t gui;
    gs_camera_t camera;
    gs_vqs xform;
    int32_t selection;
} app_t; 

// Core physics shapes
gs_aabb_t       aabb     = {0};
gs_sphere_t     sphere   = {0};
gs_cylinder_t   cylinder = {0};
gs_cone_t       cone     = {0};
gs_capsule_t    capsule  = {0};
gs_poly_t       poly     = {0}; 

enum 
{
    COLLISION_SHAPE_AABB = 0x00,
    COLLISION_SHAPE_SPHERE,
    COLLISION_SHAPE_CYLINDER,
    COLLISION_SHAPE_CONE,
    COLLISION_SHAPE_CAPSULE,
    COLLISION_SHAPE_POLY
};

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new();
    app->gsi = gs_immediate_draw_new(gs_platform_main_window());
    gs_gui_init(&app->gui, gs_platform_main_window());
    app->camera = gs_camera_perspective();
    app->xform = (gs_vqs) {
        .translation = gs_v3(0.f, 0.f, -2.f),
        .rotation = gs_quat_default(), 
        .scale = gs_v3s(1.f)
    };

    aabb = gs_aabb(.min = gs_v3s(-0.5f), .max = gs_v3s(0.5f));
    sphere = gs_sphere(.c = gs_v3s(0.f), .r = 0.5f);
    cylinder = gs_cylinder(.r = 0.5f, .base = gs_v3(0.f, 0.f, 0.f), .height = 1.f);
    cone = gs_cone(.r = 0.5f, .base = gs_v3(0.f, 0.f, 0.f), .height = 1.f);
    capsule = gs_capsule(.r = 0.5f, .base = gs_v3(0.f, 0.f, 0.f), .height = 1.f);
    poly = gs_pyramid_poly(gs_v3(-0.5f, -0.5f, -0.5f), gs_v3(0.5f, -0.5f, 0.5f), 1.f);
}

void app_update()
{
    app_t* app = gs_user_data(app_t);
    gs_immediate_draw_t* gsi = &app->gsi;
    gs_command_buffer_t* cb = &app->cb;
    gs_gui_context_t* gui = &app->gui;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const float t = gs_platform_elapsed_time();
    gs_contact_info_t res = {0};

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
        gs_quit();
    }

    // Rotate xform over time
    app->xform.rotation = gs_quat_mul_list(3, 
        gs_quat_angle_axis(t * 0.0001f, GS_XAXIS), 
        gs_quat_angle_axis(t * 0.0002f, GS_YAXIS),
        gs_quat_angle_axis(t * 0.0003f, GS_ZAXIS)
    );

    // Ray cast against various shapes in scene 
    const float ray_len = 1000.f;
    const gs_vec2 mc = gs_platform_mouse_positionv(); // Mouse coordinate
    const gs_vec3 ms = gs_v3(mc.x, mc.y, 0.f);        // Mouse coordinate start with z = 0.f
    const gs_vec3 me = gs_v3(mc.x, mc.y, -ray_len);   // Mouse coordinate end with z = -1000.f (for ray cast into screen)
    const gs_vec3 ro = gs_camera_screen_to_world(&app->camera, ms, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y);
    const gs_vec3 rd = gs_camera_screen_to_world(&app->camera, me, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y); 

    gs_ray_t ray = {
        .p = ro,
        .d = gs_vec3_norm(gs_vec3_sub(ro, rd)),
        .len = ray_len
    }; 

    // Do ray cast against selected shape
    switch (app->selection)
    {
        case COLLISION_SHAPE_AABB:      gs_aabb_vs_ray(&aabb, &app->xform, &ray, NULL, &res); break;
        case COLLISION_SHAPE_SPHERE:    gs_sphere_vs_ray(&sphere, &app->xform, &ray, NULL, &res); break;
        case COLLISION_SHAPE_CYLINDER:  gs_cylinder_vs_ray(&cylinder, &app->xform, &ray, NULL, &res); break;
        case COLLISION_SHAPE_CONE:      gs_cone_vs_ray(&cone, &app->xform, &ray, NULL, &res); break;
        case COLLISION_SHAPE_CAPSULE:   gs_capsule_vs_ray(&capsule, &app->xform, &ray, NULL, &res); break;
        case COLLISION_SHAPE_POLY:      gs_poly_vs_ray(&poly, &app->xform, &ray, NULL, &res); break;
    } 

    // Immediate draw scene
    gsi_camera(gsi, &app->camera, (uint32_t)fbs.x, (uint32_t)fbs.y);
    gsi_depth_enabled(gsi, true);
    gsi_face_cull_enabled(gsi, true);
    gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_mul_matrix(gsi, gs_vqs_to_mat4(&app->xform));
        gs_color_t col = res.hit ? GS_COLOR_RED : GS_COLOR_WHITE;

        switch (app->selection)
        { 
            case COLLISION_SHAPE_AABB:
            {
                gs_vec3 hd = gs_vec3_scale(gs_vec3_sub(aabb.max, aabb.min), 0.5f);
                gs_vec3 c = gs_vec3_add(aabb.min, hd);
                gsi_box(gsi, c.x, c.y, c.z, hd.x, hd.y, hd.z, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;

            case COLLISION_SHAPE_SPHERE:
            { 
                gs_vec3 c = sphere.c;
                float r = sphere.r;
                gsi_sphere(gsi, c.x, c.y, c.z, r, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;

            case COLLISION_SHAPE_CYLINDER:
            {
                gs_vec3 b = cylinder.base;
                float r = cylinder.r;
                float h = cylinder.height; 
                gsi_cylinder(gsi, b.x, b.y, b.z, r, r, h, 16, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;

            case COLLISION_SHAPE_CONE:
            {
                gs_vec3 b = cone.base;
                float r = cone.r;
                float h = cone.height;
                gsi_cone(gsi, b.x, b.y, b.z, r, h, 16, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;

            case COLLISION_SHAPE_CAPSULE:
            {
                    // Cylinder body
                    gsi_cylinder(gsi, 0.f, 0.f, 0.f, capsule.r, capsule.r, capsule.height, 32, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);

                    // Two spheres at ends
                    const float hh = capsule.height * 0.5f;
                    gsi_sphere(gsi, 0.f, hh, 0.f, capsule.r, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_sphere(gsi, 0.f, -hh, 0.f, capsule.r, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;

            case COLLISION_SHAPE_POLY:
            {
                gsi_pyramid(gsi, &poly, col, GS_GRAPHICS_PRIMITIVE_LINES);
            } break;
        }
    }
    gsi_pop_matrix(gsi);

    // Render gui for options
    gs_gui_begin(gui, NULL);

    int32_t op = GS_GUI_OPT_NOTITLE | 
            GS_GUI_OPT_NORESIZE | 
            GS_GUI_OPT_NOMOVE | 
            GS_GUI_OPT_FULLSCREEN | 
            GS_GUI_OPT_FORCESETRECT | 
            GS_GUI_OPT_NORESIZE | 
            GS_GUI_OPT_NOFRAME |
            GS_GUI_OPT_NOTITLE;
    gs_gui_window_begin_ex(gui, "#dbg", gs_gui_rect(0, 0, 200, 100), NULL, NULL, op);
    { 
        struct {const char* str; int32_t option;} selections[] = { 
            {"AABB", COLLISION_SHAPE_AABB},
            {"Sphere", COLLISION_SHAPE_SPHERE},
            {"Cylilnder", COLLISION_SHAPE_CYLINDER},
            {"Cone", COLLISION_SHAPE_CONE},
            {"Capsule", COLLISION_SHAPE_CAPSULE},
            {"Poly", COLLISION_SHAPE_POLY},
            {NULL}
        };

        gs_snprintfc(TMP, 256, "Selection: %s", selections[app->selection].str);
        gs_gui_layout_row(gui, 1, (int32_t[]){200}, 0);
        if (gs_gui_combo_begin(gui, "#options", TMP, 6))
        {
            gs_gui_layout_row(gui, 1, (int32_t[]){-1}, 0); 
            for (uint32_t i = 0; selections[i].str != NULL; ++i)
            {
                if (gs_gui_button(gui, selections[i].str)) {
                    app->selection = i; 
                }
            }
            gs_gui_combo_end(gui);
        }
    }
    gs_gui_window_end(gui);

    gs_gui_end(gui);

    // Render pass
    gs_graphics_renderpass_begin(cb, (gs_renderpass){0}); 
    { 
        gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05f, 0.05f, 1.f}}}; 
        gs_graphics_clear(cb, &clear);
        gs_graphics_set_viewport(cb, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y); 

        // Render all gsi
        gsi_renderpass_submit_ex(gsi, cb, (uint32_t)fbs.x, (uint32_t)fbs.y, NULL);

        // Render all gui
        gs_gui_render(gui, cb);
    } 
    gs_graphics_renderpass_end(cb); 

    gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
    app_t* app = gs_user_data(app_t);
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


