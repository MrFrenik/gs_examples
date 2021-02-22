/*================================================================
    * Copyright: 2020 John Jackson
    * marching cubes 

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

#define CHUNK_ARR_HALF_DIM  0
#define CHUNK_ARR_DIM (CHUNK_ARR_HALF_DIM * 2 + 1)

const char* comp_src =
"#version 430\n"
"uniform float u_roll;\n"
"layout(rgba32f, binding = 0) uniform image2D destTex;\n"
"layout (local_size_x = 16, local_size_y = 16) in;\n"
"void main() {\n"
    "ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n"
    "float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8 ) / 8.0);\n"
    "float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + u_roll) * 0.5;\n"
    "imageStore(destTex, storePos, vec4(1.0 - globalCoef * localCoef, globalCoef * localCoef, 0.0, 1.0));\n"
"}";

gs_command_buffer_t  cb  = {0};
gs_immediate_draw_t  gsi = {0};
fps_camera_t         fps = {0};
gs_handle(gs_graphics_texture_t)  cmptex  = {0};
gs_handle(gs_graphics_pipeline_t) cmdpip  = {0};
gs_handle(gs_graphics_shader_t)   cmpshd  = {0};
voxel_chunk_t        chunks[CHUNK_ARR_DIM * CHUNK_ARR_DIM * CHUNK_ARR_DIM] = {0};

// Try to evaluate which regions to update (and only update those)
// If you're going to use compute shaders, then why not just 

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();

    // Construct camera
    fps.cam = gs_camera_perspective();
    fps.cam.transform.position = gs_v3(-14.f, 2.f, 25.f);

    // Construct chunks
    for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
        for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
            for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    {
        uint32_t _x = x + CHUNK_ARR_HALF_DIM;
        uint32_t _y = y + CHUNK_ARR_HALF_DIM;
        uint32_t _z = z + CHUNK_ARR_HALF_DIM;
        chunks[_x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z)] = (voxel_chunk_t){.origin = gs_vec3_scale(gs_v3(x, y, z), CHUNK_WORLD_SIZE)};
    }

    chunks[0] = (voxel_chunk_t){.origin = gs_v3s(0.f)};

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
    gsi_face_cull_enabled(&gsi, false);

    // Draw ground plane
    gsi_box(&gsi, 0.f, -0.1f * 0.5f, 0.f, 32.f, 0.1f, 32.f, 100, 100, 100, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Draw them shits
    gsi_begin(&gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
        march_cubes(&gsi);
    gsi_end(&gsi);

    // Draw bounds for chunks
    for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
        for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
            for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    {
        uint32_t _x = x + CHUNK_ARR_HALF_DIM;
        uint32_t _y = y + CHUNK_ARR_HALF_DIM;
        uint32_t _z = z + CHUNK_ARR_HALF_DIM;
        uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);
        const float cwsh = (float)CHUNK_WORLD_SIZE * 0.5f;
        gs_vec3 ch = gs_vec3_add(chunks[idx].origin, gs_v3s(cwsh));
        gsi_box(&gsi, ch.x, ch.y, ch.z, cwsh, cwsh, cwsh, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    }

    // Draw 3d lines for axis at origin
    gs_vec3 origin = gs_v3s(0.f);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_XAXIS, 5.f), GS_COLOR_RED);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_YAXIS, 5.f), GS_COLOR_GREEN);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_ZAXIS, 5.f), GS_COLOR_BLUE);

    /* Render */
    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

float op_sub(float d1, float d2) {
    return gs_max(-d1, d2);
}

float op_union(float d1, float d2) {
    return gs_min(d1, d2);
}

float op_intersection(float d1, float d2) {
    return gs_max(d1, d2);
}

float op_smooth_union(float d1, float d2, float k) {
    float h = gs_clamp( 0.5f + 0.5f * (d2-d1)/k, 0.0f, 1.0f);
    return gs_interp_linear(d2, d1, h) - k*h*(1.0-h);
}

gs_vec3 max3(gs_vec3 a, gs_vec3 b) {
    return gs_v3(gs_max(a.x, b.x), gs_max(a.y, b.y), gs_max(a.z, b.z));
}

gs_vec3 abs3(gs_vec3 v) {
    return gs_v3(fabsf(v.x), fabsf(v.y), fabsf(v.z));
}

float sdf_box(gs_vec3 p, gs_vec3 b) 
{
    gs_vec3 q = gs_vec3_sub(abs3(p), b);
    return gs_vec3_len(max3(q, gs_v3s(0.f))) + gs_min(gs_max(q.x, gs_max(q.y, q.z)), 0.f);
}

float sdf_sphere(gs_vec3 p, float r)
{
    return (gs_vec3_len(p) - r);
}

float sdf_torus(gs_vec3 p, gs_vec2 t)
{
    gs_vec2 q = gs_v2(gs_vec2_len(gs_v2(p.x, p.z)) - t.x, p.y); 
    return gs_vec2_len(q) - t.y;
}

#define NUM_SPHERES 50
gs_vec3 sphere_pos[NUM_SPHERES] = {0};
gs_mat4 tpmat = {0};
gs_vec3 tpos = {0};

void tick_scene()
{
    const float t = gs_platform_elapsed_time();
    for (uint32_t i = 0; i < NUM_SPHERES; ++i) {
        float x = sin(t * 0.0001f + i * 0.1f); 
        float y = cos(t * 0.0002f + i * 0.1f);
        float z = sin(t * 0.0003f + i * 0.1f);
        sphere_pos[i] = gs_vec3_scale(gs_vec3_add(gs_vec3_scale(gs_v3(x, y, z), 0.5f), gs_v3s(0.5f)), CHUNK_WORLD_SIZE);
    }

    tpmat = gs_mat4_identity();
    tpmat = gs_mat4_mul_list(
        3,
        gs_mat4_rotatev(t * 0.002f, GS_XAXIS),
        gs_mat4_rotatev(t * 0.001f, GS_YAXIS),
        gs_mat4_rotatev(t * 0.003f, GS_ZAXIS)
    );
    gs_vec4 tp = gs_v4(8.f, 8.f, 8.f, 1.f);
    tpos = gs_v3(tp.x, tp.y, tp.z);
}

float scene(gs_vec3 p)
{
    float dist = FLT_MAX;
    for (uint32_t i = 0; i < NUM_SPHERES; ++i) {
        dist = op_smooth_union(dist, sdf_sphere(gs_vec3_sub(sphere_pos[i], p), (float)i / (float)NUM_SPHERES + 0.1f), 0.5f);
    }

    float b0 = sdf_box(gs_vec3_sub(gs_v3(8.f, 5.f, 4.f), p), gs_v3(5.f, 5.f, 1.f));
    float t0 = sdf_torus(gs_mat4_mul_vec3(tpmat, gs_vec3_sub(tpos, p)), gs_v2(5.f, 1.f));
    // float b1 = sdf_box(gs_vec3_sub(gs_v3(10.f, 5.f, 2.f), p), gs_v3(10.f, 5.f, 2.f));

    const float t = gs_platform_elapsed_time();
    // dist = op_union(dist, b0); 
    dist = op_smooth_union(dist, t0, 1.f); 
    // dist = op_union(dist, b1);

    return dist;
}

//fGetOffset finds the approximate point of intersection of the surface
// between two points with the values fValue1 and fValue2
float fGetOffset(float fValue1, float fValue2, float fValueDesired)
{
    double fDelta = fValue2 - fValue1;
    if(fDelta == 0.0) {
        return 0.5;
    }
    return (fValueDesired - fValue1)/fDelta;
}

//vGetNormal() finds the gradient of the scalar field at a point
//This gradient can be used as a very accurate vertx normal for lighting calculations
gs_vec3 get_normal(float fX, float fY, float fZ)
{
    gs_vec3 n = {0};
    n.x = scene(gs_v3(fX-0.01f, fY, fZ)) - scene(gs_v3(fX+0.01f, fY, fZ));
    n.y = scene(gs_v3(fX, fY-0.01f, fZ)) - scene(gs_v3(fX, fY+0.01f, fZ));
    n.z = scene(gs_v3(fX, fY, fZ-0.01f)) - scene(gs_v3(fX, fY, fZ+0.01f));
    return gs_vec3_norm(n);
}

//vGetColor generates a color from a given position and normal of a point
gs_color_t get_color(gs_vec3 normal)
{
    gs_vec3 norm = gs_vec3_scale(gs_vec3_add(gs_vec3_scale(gs_vec3_norm(normal), 0.5f), gs_v3s(0.5f)), 255.f);
    return gs_color((uint8_t)norm.x, (uint8_t)norm.y, (uint8_t)norm.z, 255); 
}

void march_single_cube(gs_immediate_draw_t* gsi, gs_vec3 p, float scale)
{
    int32_t iCorner, iVertex, iVertexTest, iEdge, iTriangle, iFlagIndex, iEdgeFlags;
    float fOffset;
    gs_color_t sColor;
    float afCubeValue[8];
    gs_vec3 asEdgeVertex[12];
    gs_vec3 asEdgeNorm[12];

    //Make a local copy of the values at the cube's corners
    for (iVertex = 0; iVertex < 8; iVertex++) {
        afCubeValue[iVertex] = scene(gs_v3(p.x + a2fVertexOffset[iVertex][0] * scale,
                                           p.y + a2fVertexOffset[iVertex][1] * scale,
                                           p.z + a2fVertexOffset[iVertex][2] * scale));
    }

    //Find which vertices are inside of the surface and which are outside
    const float fTargetValue = 0.5f; // Still not sure about this, but for some reason it works?

    iFlagIndex = 0;
    for(iVertexTest = 0; iVertexTest < 8; iVertexTest++) {
        if(afCubeValue[iVertexTest] <= fTargetValue) 
                iFlagIndex |= 1<<iVertexTest;
    }

    //Find which edges are intersected by the surface
    iEdgeFlags = aiCubeEdgeFlags[iFlagIndex];

    //Find the point of intersection of the surface with each edge
    //Then find the normal to the surface at those points
    for (iEdge = 0; iEdge < 12; iEdge++)
    {
        //if there is an intersection on this edge
        if(iEdgeFlags & (1<<iEdge))
        {
            fOffset = fGetOffset(afCubeValue[a2iEdgeConnection[iEdge][0]], afCubeValue[a2iEdgeConnection[iEdge][1]], fTargetValue);

            asEdgeVertex[iEdge].x = p.x + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][0] + fOffset * a2fEdgeDirection[iEdge][0]) * scale;
            asEdgeVertex[iEdge].y = p.y + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][1] + fOffset * a2fEdgeDirection[iEdge][1]) * scale;
            asEdgeVertex[iEdge].z = p.z + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][2] + fOffset * a2fEdgeDirection[iEdge][2]) * scale;

            asEdgeNorm[iEdge] = get_normal(asEdgeVertex[iEdge].x, asEdgeVertex[iEdge].y, asEdgeVertex[iEdge].z);
        }
    }

    //Draw the triangles that were found.  There can be up to five per cube
    for (iTriangle = 0; iTriangle < 5; iTriangle++)
    {
        if (a2iTriangleConnectionTable[iFlagIndex][3*iTriangle] < 0)
                break;

        for (iCorner = 0; iCorner < 3; iCorner++)
        {
            iVertex = a2iTriangleConnectionTable[iFlagIndex][3*iTriangle+iCorner];

            sColor = get_color(asEdgeNorm[iVertex]);
            gsi_c4ub(gsi, sColor.r, sColor.g, sColor.b, sColor.a);
            gsi_v3fv(gsi, asEdgeVertex[iVertex]);
        }
    }
}

void march_cubes(gs_immediate_draw_t* gsi)
{
    // Update scene stuff
    tick_scene();

    // For each chunk
    for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
        for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
            for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    {
        uint32_t _x = x + CHUNK_ARR_HALF_DIM;
        uint32_t _y = y + CHUNK_ARR_HALF_DIM;
        uint32_t _z = z + CHUNK_ARR_HALF_DIM;
        uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);

        // Determine resolution based on camera distance
        gs_vec3 diff = gs_vec3_sub(fps.cam.transform.position, chunks[idx].origin);
        float sdist = gs_vec3_dot(diff, diff);
        if (sdist <= 600.f)       chunks[idx].resolution = 0.5f;
        else if (sdist <= 1500.f) chunks[idx].resolution = 1.f;
        else                      chunks[idx].resolution = 2.f;

        const uint32_t NUM_VOXELS = (uint32_t)((float)CHUNK_WORLD_SIZE / chunks[idx].resolution);
        const float scale = (float)chunks[idx].resolution;

        // For each voxel
        for (uint32_t ix = 0; ix < NUM_VOXELS; ++ix)
            for (uint32_t iy = 0; iy < NUM_VOXELS; ++iy)
                for (uint32_t iz = 0; iz < NUM_VOXELS; ++iz)
        {
            // Calculate position of voxel using resolution scale and chunk origin   
            gs_vec3 p = gs_vec3_add(chunks[idx].origin, gs_v3(ix * scale, iy * scale, iz * scale));

            // Then march single cube evaluating whatever scalar field you want to use.
            march_single_cube(gsi, p, scale);
        }
    }
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
        .update = app_update,
        .window_width = 1920,
        .window_height = 1080
    };
}   






