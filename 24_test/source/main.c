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
gs_asset_mesh_t duck = {0};
gs_asset_texture_t tex = {0};
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
    fps.cam.fov = 60.f;

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

#define rand_range(MIN, MAX)\
    (rand() % (MAX - MIN + 1) + MIN)

    for (uint32_t i = 0; i < 100; ++i)
    {
        gs_vec3 p = gs_v3(
            (float)rand_range(-20, 20),
            (float)rand_range(-20, 20),
            (float)rand_range(-20, 20)
        ); 
        gs_dyn_array_push(positions, p);
    }
}

gs_vec3 toEuler(double x,double y,double z,double angle) {
    gs_vec3 euler = {0};
    float heading = 0.f, attitude = 0.f, bank = 0.f;
    double s=sinf(angle);
    double c=cosf(angle);
    double t=1-c;
    //  if axis is not already normalised then uncomment this
    // double magnitude = Math.sqrt(x*x + y*y + z*z);
    // if (magnitude==0) throw error;
    // x /= magnitude;
    // y /= magnitude;
    // z /= magnitude;
    if ((x*y*t + z*s) > 0.998) { // north pole singularity detected
        heading = 2*atan2(x*sinf(angle/2),cosf(angle/2));
        attitude = (float)GS_PI/2.f;
        bank = 0;
        euler = gs_v3(heading, attitude, bank);
        return euler;
    }
    if ((x*y*t + z*s) < -0.998) { // south pole singularity detected
        heading = -2*atan2(x*sinf(angle/2),cosf(angle/2));
        attitude = (float)-GS_PI/2;
        bank = 0;
        euler = gs_v3(heading, attitude, bank);
        return euler;
    }
    heading = atan2f(y * s- x * z * t , 1 - (y*y+ z*z ) * t);
    attitude = asinf(x * y * t + z * s) ;
    bank = atan2f(x * s - y * z * t , 1 - (x*x + z*z) * t);
    euler = gs_v3(heading, attitude, bank);
    return euler;
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

    // Calculate angle between, then mod/clamp each element to set rotation
    gs_mat4 rot = {0};
    const uint32_t ct = 8;
    const float step = 360.f / (float)ct;

    // Draw simple cube with forward vector
    gsi_camera(&gsi, &fps.cam);
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, false);

    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    {
        // Calculate "lookat" billboard matrix
        const float t = gs_platform_elapsed_time();
        gs_mat4 cr = gs_mat4_inverse(gs_quat_to_mat4(fps.cam.transform.rotation));
        gs_vec3 pos = gs_v3(0.f, 1.f, -2.f);
        gs_vec3 target = gs_vec3_norm(gs_vec3_sub(fps.cam.transform.position, pos));
        const float s2 = step * 0.5f;

        // Y radians
        static gs_quat tr = {0};
        static bool done = false;
        if (!done) {
            done = true;
            tr = gs_quat_default();
        } 
        tr = gs_quat_angle_axis(t * 0.001f, GS_YAXIS);
        tr = gs_quat_default();
        gs_vec3 fw = gs_quat_rotate(tr, GS_ZAXIS);

        // float yfwa = fmodf(gs_rad2deg((atan2f(fw.z, fw.x) + gs_deg2rad(90.f))) + 180.f, 360.f);
        float yfwa = fmodf(gs_rad2deg((atan2f(fw.z, fw.x))) + 90.f, 360.f);
        float yangle = atan2f(target.z, target.x);
        float ydeg = fmodf(gs_rad2deg(yangle) + 360.f, 360.f);

        // Just want to figure out "extra" amount to billboard to based on where camera is from step value
        float yoff  = floor((yfwa + s2) / step) * step;
        float sydeg = floor((ydeg + s2) / step) * step;
        float rota = fmodf((ydeg - sydeg + yoff) + 180.f, 360.f);

        // gsi_mul_matrix(&gsi, cr);
        gsi_transf(&gsi, pos.x, pos.y, pos.z);
        gsi_rotatefv(&gsi, -gs_deg2rad(rota), GS_YAXIS);

        gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
        gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_XAXIS, GS_COLOR_RED);
        gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_YAXIS, GS_COLOR_GREEN);
        gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_ZAXIS, GS_COLOR_BLUE);
    }
    gsi_pop_matrix(&gsi);

    gsi_transf(&gsi, 2.f, 0.f, -2.f);
    gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_line3Dv(&gsi, gs_v3(0.f, 0.f, 0.f), GS_ZAXIS, GS_COLOR_BLUE);

    const gs_vec2 fb = gs_platform_framebuffer_sizev(0);
    const gs_vec2 ws = gs_platform_window_sizev(0);

    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 255}};

    // Bind render pass for backbuffer
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&cb, 0, 0, (int32_t)fb.x, (int32_t)fb.y);
        gs_graphics_clear(&cb, &clear);
        gsi_draw(&gsi, &cb); // Binds pipelines and submits to graphics command buffer for rendering

        // Get pipeline for mesh from immediate draw backend
        gsi_pipeline_state_attr_t state = {
            .depth_enabled = 1,
            .stencil_enabled = 0,
            .blend_enabled = 0,
            .face_cull_enabled = 1,
            .prim_type = (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES
        };
        gs_handle(gs_graphics_pipeline_t) pip = gsi_get_pipeline(&gsi, state);

        gs_graphics_bind_pipeline(&cb, pip); // Bind pipeline

        // MVP Matrix
        gs_mat4 vp  = gs_camera_get_view_projection(&fps.cam, (int32_t)ws.x, (int32_t)ws.y);
        // gs_mat4 mvp = gs_mat4_perspective(60.f, ws.x / ws.y, 0.1f, 1000.f);

        for (uint32_t j = 0; j < gs_dyn_array_size(positions); ++j) 
        {
            gs_mat4 mvp = vp;

            // Calculate "lookat" billboard matrix
            const float t = gs_platform_elapsed_time();
            gs_vec3 pos = positions[j];
            gs_vec3 target = gs_vec3_sub(fps.cam.transform.position, pos);
            // gs_mat4 lookat = gs_mat4_inverse(gs_mat4_look_at(pos, gs_v3(target.x, pos.y, target.z), GS_YAXIS));
            const float s2 = step * 0.5f;

            // Y radians
            static gs_quat tr = {0};
            static bool done = false;
            if (!done) {
                done = true;
                tr = gs_quat_default();
            } 
            tr = gs_quat_angle_axis(t * 0.001f, GS_YAXIS);
            tr = gs_quat_default();
            gs_vec3 fw = gs_quat_rotate(tr, GS_ZAXIS);

            float yfwa = fmodf(gs_rad2deg((atan2f(fw.z, fw.x))) + 360.f, 360.f);
            float yangle = atan2f(target.z, target.x);
            float ydeg = fmodf(gs_rad2deg(yangle) + 360.f, 360.f);

            // Just want to figure out "extra" amount to billboard to based on where camera is from step value
            float yoff  = floor((yfwa + s2) / step) * step;
            float sydeg = floor((ydeg + s2) / step) * step;
            float rota = fmodf((ydeg - sydeg + yoff) + 180.f, 360.f);

            mvp = gs_mat4_mul_list(
                4, 
                mvp,
                gs_mat4_translate(pos.x, pos.y, pos.z),
                gs_mat4_rotatev(-gs_deg2rad(rota), GS_YAXIS), 
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
        }

    gs_graphics_end_render_pass(&cb);

    // gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));
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
    // if (gs_vec3_len(vel) != 0.f) {
    //     fps->bob_time += dt * 8.f;
    //     float sb = sin(fps->bob_time);
    //     float bob_amt = (sb * 0.5f + 0.5f) * 0.1f * mod;
    //     float rot_amt = sb * 0.0004f * mod;
    //     fps->cam.transform.position.y = 2.f + bob_amt;        
    //     fps->cam.transform.rotation = gs_quat_mul(fps->cam.transform.rotation, gs_quat_angle_axis(rot_amt, GS_ZAXIS));
    // }
}
