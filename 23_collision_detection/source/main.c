/*================================================================
    * Copyright: 2020 John Jackson
    * simple collision detection example

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_PHYSICS_IMPL
#include <gs/util/gs_physics.h>

#include "data.c"

#define MANIFOLD_CONTACT_POINTS 1

typedef struct manifold_t {
    uint32_t ct, idx;
    gs_gjk_contact_info_t pts[MANIFOLD_CONTACT_POINTS];
} manifold_t;

typedef enum shape_selection {
    SHAPE_SELECTION_SPHERE = 0x00,
    SHAPE_SELECTION_AABB, 
    SHAPE_SELECTION_PYRAMID,
    SHAPE_SELECTION_TRIANGLE,
    SHAPE_SELECTION_CYLINDER,
    SHAPE_SELECTION_CONE,
    SHAPE_SELECTION_CAPSULE,
    SHAPE_SELECTION_QUAD,
    SHAPE_SELECTION_COUNT
} shape_selection;

gs_command_buffer_t  cb  = {0};
gs_immediate_draw_t  gsi = {0};

// Physics shapes
gs_aabb_t       aabb     = {0};
gs_sphere_t     sphere   = {0};
gs_poly_t       pyramid  = {0};
gs_triangle_t   triangle = {0};
gs_cylinder_t   cylinder = {0};
gs_cone_t       cone     = {0};
gs_capsule_t    capsule  = {0};
gs_quad_t       quad     = {0};

// Transforms
gs_vqs transforms[2] = {0};
bool transform_enabled = true;

gs_vqs default_xform = {0};

// Selected shapes
shape_selection shapes[2] = {0};

// Manifold
manifold_t manifold = {0};

// Time controls
float t = 0.f;
float tmul = 0.2f;
bool32 running = true;
bool32 render_display_info = true;

// Rendering
gs_graphics_primitive_type rendering_type = GS_GRAPHICS_PRIMITIVE_LINES;

#define GS_CONTACT_FUNC(T, OBJ)\
    {\
        switch (shapes[1]) {\
            case SHAPE_SELECTION_SPHERE:    gs_##T##_vs_sphere(&OBJ, t0, &sphere, t1, &info); break;\
            case SHAPE_SELECTION_AABB:      gs_##T##_vs_aabb(&OBJ, t0, &aabb, t1, &info); break;\
            case SHAPE_SELECTION_PYRAMID:   gs_##T##_vs_poly(&OBJ, t0, &pyramid, t1, &info); break;\
            case SHAPE_SELECTION_TRIANGLE:  gs_##T##_vs_triangle(&OBJ, t0, &triangle, t1, &info); break;\
            case SHAPE_SELECTION_CYLINDER:  gs_##T##_vs_cylinder(&OBJ, t0, &cylinder, t1, &info); break;\
            case SHAPE_SELECTION_CONE:      gs_##T##_vs_cone(&OBJ, t0, &cone, t1, &info); break;\
            case SHAPE_SELECTION_CAPSULE:   gs_##T##_vs_capsule(&OBJ, t0, &capsule, t1, &info); break;\
            case SHAPE_SELECTION_QUAD:      gs_##T##_vs_quad(&OBJ, t0, &quad, t1, &info); break;\
        }\
    }

gs_gjk_contact_info_t app_do_collisions()
{
    // Depending on whether or not transform 0 is enabled, we'll use the transform or a null pointer
    gs_vqs* t0 = transform_enabled ? &transforms[0] : &default_xform;
    gs_vqs* t1 = &transforms[1];

    gs_gjk_contact_info_t info = {0};
    switch (shapes[0])
    {
        case SHAPE_SELECTION_SPHERE:    GS_CONTACT_FUNC(sphere, sphere); break;
        case SHAPE_SELECTION_AABB:      GS_CONTACT_FUNC(aabb, aabb);    break; 
        case SHAPE_SELECTION_PYRAMID:   GS_CONTACT_FUNC(poly, pyramid); break; 
        case SHAPE_SELECTION_TRIANGLE:  GS_CONTACT_FUNC(triangle, triangle); break;
        case SHAPE_SELECTION_CYLINDER:  GS_CONTACT_FUNC(cylinder, cylinder); break; 
        case SHAPE_SELECTION_CONE:      GS_CONTACT_FUNC(cone, cone); break;
        case SHAPE_SELECTION_CAPSULE:   GS_CONTACT_FUNC(capsule, capsule); break;
        case SHAPE_SELECTION_QUAD:      GS_CONTACT_FUNC(quad, quad); break;
    }

    return info;
}

const char* shape_to_str(shape_selection sel)
{
    switch (sel)
    {
        default: 
        case SHAPE_SELECTION_COUNT:     return "invalid"; break;
        case SHAPE_SELECTION_SPHERE:    return "sphere"; break;
        case SHAPE_SELECTION_PYRAMID:   return "pyramid"; break;
        case SHAPE_SELECTION_AABB:      return "aabb"; break;
        case SHAPE_SELECTION_TRIANGLE:  return "triangle"; break;
        case SHAPE_SELECTION_CYLINDER:  return "cylinder"; break;
        case SHAPE_SELECTION_CONE:      return "cone"; break;
        case SHAPE_SELECTION_CAPSULE:   return "capsule"; break;
        case SHAPE_SELECTION_QUAD:      return "quad"; break;
    }
    return "invalid";
}

void reset_manifold(manifold_t* manifold)
{
    manifold->ct = 0;
    manifold->idx = 0;
}

void update_manifold(manifold_t* manifold, gs_gjk_contact_info_t* info)
{
    // Add a new contact point, update point
    if (info->hit) 
    {
        manifold->idx = (manifold->idx + 1) % MANIFOLD_CONTACT_POINTS;
        manifold->ct = gs_clamp(manifold->ct + 1, 0, MANIFOLD_CONTACT_POINTS);
        manifold->pts[manifold->idx] = *info;
    } 
    else 
    {
        reset_manifold(manifold); 
    }
}

bool manifold_is_valid(manifold_t* manifold)
{
    return (manifold->ct >= MANIFOLD_CONTACT_POINTS);
}

void app_do_input()
{
    // Quit engine/app
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Alternate shape selections
    if (gs_platform_key_pressed(GS_KEYCODE_M)) {shapes[0] = (shapes[0] + 1) % SHAPE_SELECTION_COUNT; reset_manifold(&manifold);}
    if (gs_platform_key_pressed(GS_KEYCODE_N)) {shapes[1] = (shapes[1] + 1) % SHAPE_SELECTION_COUNT; reset_manifold(&manifold);} 

    // Speed up / Slow time
    if (gs_platform_key_pressed(GS_KEYCODE_E)) tmul += 0.1f;
    if (gs_platform_key_pressed(GS_KEYCODE_Q)) tmul -= 0.1f;

    // Pause time
    if (gs_platform_key_pressed(GS_KEYCODE_P)) running = !running;

    // Disable transform 0
    if (gs_platform_key_pressed(GS_KEYCODE_T)) transform_enabled = !transform_enabled;

    // Disable display info
    if (gs_platform_key_pressed(GS_KEYCODE_D)) render_display_info = !render_display_info;

    // Alternate rendering style
    if (gs_platform_key_pressed(GS_KEYCODE_R)) {
        rendering_type = (rendering_type == GS_GRAPHICS_PRIMITIVE_LINES ? GS_GRAPHICS_PRIMITIVE_TRIANGLES : GS_GRAPHICS_PRIMITIVE_LINES);
    }
}

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();

    // There's a bug with cylinder vs. sphere if cylinder and sphere centers/bases are both at origin
    // Initialize all necessary collision shapes
    aabb = gs_aabb(.min = gs_v3s(-0.5f), .max = gs_v3s(0.5f));
    sphere = gs_sphere(.c = gs_v3s(0.f), .r = 1.f);
    pyramid = gs_pyramid_poly(gs_v3s(0.f), gs_v3(0.f, 2.f, 0.f), 1.f);
    triangle = gs_triangle(.a = gs_v3(-0.5f, -0.5f, 0.f), .b = gs_v3(0.5f, -0.5f, 0.f), .c = gs_v3(0.f, 0.5f, 0.f));
    cylinder = gs_cylinder(.r = 0.5f, .base = gs_v3(1.f, 0.f, 0.f), .height = 3.f);
    cone = gs_cone(.r = 0.5f, .base = gs_v3(0.f, 0.f, 0.f), .height = 3.f);
    capsule = gs_capsule(.r = 0.5f, .base = gs_v3s(0.f), .height = 2.f);
    quad = gs_quad(.min = gs_v2s(-0.5f), .max = gs_v2s(0.5f));

    default_xform = gs_vqs_default();
}

void app_update()
{
    // Increment time
    if (running) t += gs_platform_delta_time() * tmul;

    // Update inputs
    app_do_input();

    // Immediate draw scene
    gsi_camera3D(&gsi);
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, true);

    // Transform scene camera
    gsi_transf(&gsi, 0.f, 0.f, -5.f);

    // All physics collision detection calls can accept optional transform information.
    // These transforms will allow you to rotate/translate/scale (uniformly, if spheres) your objects
    // before performing collision detection/resolution code on them. We'll create two separate transforms
    // here to use for collisions.
    transforms[0] = (gs_vqs){
        .position = gs_v3(1.f, sin(t) * 2.1f, 0.f),
        .rotation = gs_quat_mul_list(3,
            gs_quat_default(),
            gs_quat_default(),
            // gs_quat_angle_axis(t * 2.f,  GS_XAXIS),
            // gs_quat_angle_axis(t * 0.5f, GS_YAXIS),
            gs_quat_angle_axis(t * 3.f,  GS_ZAXIS)
        ),
        .scale = gs_v3s(1.f)
    };

    transforms[1] = (gs_vqs){
        .position = gs_v3(1.f, sin(t * 0.5f) * -2.5f, 0.f),
        .rotation = gs_quat_mul_list(3, 
            gs_quat_default(),
            gs_quat_default(),
            // gs_quat_angle_axis(t * 5.f,  GS_XAXIS),
            // gs_quat_angle_axis(t * 1.5f, GS_YAXIS),
            gs_quat_angle_axis(t * 0.4f, GS_ZAXIS)
        ),
        .scale = gs_v3s(1.f)
    };

    // Detect two shapes based on selections
    gs_gjk_contact_info_t ci = app_do_collisions();

    // Update manifold with current info
    update_manifold(&manifold, &ci);

    // Render shapes
    const gs_color_t col = manifold_is_valid(&manifold) ? GS_COLOR_RED : GS_COLOR_GREEN;

    // Cache transform pointers for rendering
    gs_vqs* xforms[2] = {
        transform_enabled ? &transforms[0] : &default_xform,
        &transforms[1]
    };

    // Render each collision shape
    for (uint32_t i = 0; i < 2; ++i)
    {
        gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
        {
            // If transform, use it 
            if (xforms[i]) gsi_mul_matrix(&gsi, gs_vqs_to_mat4(xforms[i]));

            switch (shapes[i]) 
            {
                case SHAPE_SELECTION_SPHERE:  
                {
                    gsi_sphere(&gsi, sphere.c.x, sphere.c.y, sphere.c.z, sphere.r, col.r, col.g, col.b, col.a, rendering_type);
                } break;

                case SHAPE_SELECTION_PYRAMID:  
                {
                    gsi_pyramid(&gsi, &pyramid, col, rendering_type);
                } break;

                case SHAPE_SELECTION_AABB:  
                {
                    gs_vec3 hd = gs_vec3_scale(gs_vec3_sub(aabb.max, aabb.min), 0.5f);
                    gs_vec3 c = gs_vec3_add(aabb.min, hd);
                    gsi_box(&gsi, c.x, c.y, c.z, hd.x, hd.y, hd.z, col.r, col.g, col.b, col.a, rendering_type);
                } break;

                case SHAPE_SELECTION_TRIANGLE: 
                {
                    gs_vec3* a = &triangle.a;
                    gs_vec3* b = &triangle.b;
                    gs_vec3* c = &triangle.c;
                    gsi_trianglevx(&gsi, *a, *b, *c, gs_v2s(0.f), gs_v2s(1.f), gs_v2s(0.5f), col, rendering_type);
                } break;

                case SHAPE_SELECTION_CYLINDER: 
                {
                    // Need to draw a cylinder with gsi
                    gsi_cylinder(&gsi, cylinder.base.x, cylinder.base.y, cylinder.base.z, 
                        cylinder.r, cylinder.r, cylinder.height, 32, col.r, col.g, col.b, col.a, rendering_type);
                } break;

                case SHAPE_SELECTION_CONE: 
                {
                    // Need to draw a cylinder with gsi
                    gsi_cone(&gsi, cone.base.x, cone.base.y, cone.base.z, 
                        cone.r, cone.height, 32, col.r, col.g, col.b, col.a, rendering_type); 
                } break;

                case SHAPE_SELECTION_CAPSULE: 
                {
                    // Cylinder body
                    gsi_cylinder(&gsi, capsule.base.x, capsule.base.y, capsule.base.z, 
                        capsule.r, capsule.r, capsule.height, 32, col.r, col.g, col.b, col.a, rendering_type); 

                    // Two spheres at ends
                    gs_vec3 sp0 = capsule.base;
                    gs_vec3 sp1 = gs_vec3_add(capsule.base, gs_v3(0.f, capsule.height, 0.f));
                    gsi_sphere(&gsi, sp0.x, sp0.y, sp0.z, capsule.r, col.r, col.g, col.b, col.a, rendering_type);
                    gsi_sphere(&gsi, sp1.x, sp1.y, sp1.z, capsule.r, col.r, col.g, col.b, col.a, rendering_type);
                } break;

                case SHAPE_SELECTION_QUAD: 
                {
                    gsi_rectvd(&gsi, quad.min, gs_vec2_sub(quad.max, quad.min), gs_v2s(0.f), gs_v2s(1.f), col, rendering_type);
                } break;
            }
        }
        gsi_pop_matrix(&gsi);
    }

    // Render collision info (only if manifold is valid)
    if (manifold_is_valid(&manifold))
    {
        for (uint32_t i = 0; i < gs_min(manifold.ct, MANIFOLD_CONTACT_POINTS); ++i)
        {
            gs_gjk_contact_info_t* info = &manifold.pts[i];  

            // Cache pointers
            gs_vec3* p0 = &info->points[0];
            gs_vec3* p1 = &info->points[1];
            gs_vec3* n = &info->normal;

            // Contact points (one for each collider)
            gsi_sphere(&gsi, p0->x, p0->y, p0->z, 0.1f, 0, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES);
            gsi_sphere(&gsi, p1->x, p1->y, p1->z, 0.1f, 0, 255, 0, 255, GS_GRAPHICS_PRIMITIVE_LINES);

            // Normal from contact point 0
            gs_vec3 e = gs_vec3_sub(*p0, gs_vec3_scale(*n, info->depth));
            gsi_line3Dv(&gsi, *p0, e, gs_color(255, 255, 0, 255));

            // Normal from contact point 1
            e = gs_vec3_add(*p1, gs_vec3_scale(*n, info->depth));
            gsi_line3Dv(&gsi, *p1, e, gs_color(255, 255, 0, 255));
        }
    }

    // Render app control info
    if (render_display_info)
    {
        gsi_camera2D(&gsi);
        gsi_defaults(&gsi);

        // Text box
        gsi_rectvd(&gsi, gs_v2(10.f, 10.f), gs_v2(380.f, 300.f), gs_v2s(0.f), gs_v2s(1.f), gs_color(25, 25, 25, 200), GS_GRAPHICS_PRIMITIVE_TRIANGLES);

        // Text
        gsi_text(&gsi, 15.f, 25.f, "Controls:", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 40.f, "- Quit: ESC", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 55.f, "- M: Change shape selection 0", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 70.f, "- N: Change shape selection 1", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 85.f, "- Q: Decrement time", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 100.f, "- E: Increment time", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 115.f, "- P: Pause time", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 130.f, "- T: Disable/Enable transform 0", NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 145.f, "- D: Disable/Enable display info", NULL, false, 255, 255, 255, 255);

        // Display shape info
        gs_snprintfc(s0, 256, "- Shape 0: %s", shape_to_str(shapes[0]));
        gs_snprintfc(s1, 256, "- Shape 1: %s", shape_to_str(shapes[1]));
        gsi_text(&gsi, 25.f, 160.f, s0, NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 175.f, s1, NULL, false, 255, 255, 255, 255);

        // Display collision info
        if (manifold_is_valid(&manifold)) {
            gs_gjk_contact_info_t* info = &manifold.pts[0];
            gs_vec3* p0 = &info->points[0];
            gs_vec3* p1 = &info->points[1];
            gs_vec3* n = &info->normal;
            float* d = &info->depth;
            gs_snprintfc(sp0, 256, "- Collision Point 0: <%.2f, %.2f, %.2f>", p0->x, p0->y, p0->z);
            gs_snprintfc(sp1, 256, "- Collision Point 1: <%.2f, %.2f, %.2f>", p1->x, p1->y, p1->z);
            gs_snprintfc(sn, 256, "- Collision Normal: <%.2f, %.2f, %.2f>", n->x, n->y, n->z);
            gs_snprintfc(sd, 256, "- Collision Depth: %.2f", *d);
            gs_snprintfc(sh, 256, "- Collision: %s", info->hit ? "hit" : "none");
            gsi_text(&gsi, 25.f, 190.f, sp0, NULL, false, 255, 255, 255, 255);
            gsi_text(&gsi, 25.f, 205.f, sp1, NULL, false, 255, 255, 255, 255);
            gsi_text(&gsi, 25.f, 220.f, sn, NULL, false, 255, 255, 255, 255);
            gsi_text(&gsi, 25.f, 235.f, sd, NULL, false, 255, 255, 255, 255);
            gsi_text(&gsi, 25.f, 250.f, sh, NULL, false, 255, 255, 255, 255);
        }
        else {
            gsi_text(&gsi, 25.f, 190.f, "- No Collisions", NULL, false, 255, 255, 255, 255);
        }

        // Display time
        gs_snprintfc(st, 256, "- Time: %.2f", t);
        gs_snprintfc(stm, 256, "- Time Multiplier: %.2f", tmul);
        gsi_text(&gsi, 25.f, 265.f, st, NULL, false, 255, 255, 255, 255);
        gsi_text(&gsi, 25.f, 280.f, stm, NULL, false, 255, 255, 255, 255);

        // Rendering style
        gsi_text(&gsi, 25.f, 295.f, "- R: Alternate rendering style", NULL, false, 255, 255, 255, 255);
    }

    // Final submit to immediate draw
    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));

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


