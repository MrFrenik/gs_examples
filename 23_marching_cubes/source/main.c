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

#define CHUNK_ARR_HALF_DIM  1
#define CHUNK_ARR_DIM (CHUNK_ARR_HALF_DIM * 2 + 1)
#define TOTAL_CHUNKS (CHUNK_ARR_DIM * CHUNK_ARR_DIM * CHUNK_ARR_DIM)

gs_command_buffer_t  cb  = {0};
gs_immediate_draw_t  gsi = {0};
fps_camera_t         fps = {0};
voxel_world_t        world = {0};

void tick_scene();
void build_voxel_data(voxel_chunk_t* chunk);
uint8_t quantize_float(float v, float min, float max);
float scene(gs_vec3 p);
void build_scene(voxel_world_t* world);

/*
    How do?

    Chunks: 
        * Build once, forget until requesting to place voxel structure into world, like sphere or torus
        * Try to avoid updating entire chunk if possible (just regions that are necessary)
        * Is it possible to only rebuild portions of the mesh? Seems unlikely, given how everything has to be evaluated.
*/

#define get_voxel_idx(X, Y, Z)\
    ((uint32_t)((uint32_t)(X) + NUM_CORNERS * ((uint32_t)(Y) + NUM_CORNERS * (uint32_t)(Z))))

int32_t get_chunk_idx(voxel_world_t* world, gs_vec3 p)
{
    // Need to floor these here
    p.x = floor(p.x / CHUNK_WORLD_SIZE);
    p.y = floor(p.y / CHUNK_WORLD_SIZE);
    p.z = floor(p.z / CHUNK_WORLD_SIZE);

    int32_t idx = -1;
    if (gs_hash_table_key_exists(world->chunk_map, p)) {
        idx = gs_hash_table_get(world->chunk_map, p);
    }
    return idx;
}

// K, time to re-eval
// Instead, have chunks that are available, but then be able to place voxel data into chunk and rebuild mesh on demand.

void build_voxel_data(voxel_chunk_t* chunk)
{
    gs_dyn_array_clear(chunk->dirty_list);
    memset(chunk->data, 255, NUM_CORNERS * NUM_CORNERS * NUM_CORNERS);

    const float scale = (float)CHUNK_WORLD_SIZE / (float)NUM_VOXELS;

    // Iterate over and build voxel data
    for (uint32_t ix = 0; ix < NUM_CORNERS; ++ix)
        for (uint32_t iy = 0; iy < NUM_CORNERS; ++iy)
            for (uint32_t iz = 0; iz < NUM_CORNERS; ++iz)
    {
        // Calculate position of voxel using resolution scale and chunk origin   
        gs_vec3 p = gs_vec3_add(chunk->origin, gs_v3(ix * scale, iy * scale, iz * scale));

        // Evaluate corner data at idx
        float v = scene(gs_v3(p.x + a2fVertexOffset[0][0] * scale,
                                           p.y + a2fVertexOffset[0][1] * scale,
                                           p.z + a2fVertexOffset[0][2] * scale));
        if (v <= 10.f) {
            chunk->data[get_voxel_idx(ix, iy, iz)] = quantize_float(v, 0.f, 1.f);
            if (ix != NUM_VOXELS && iy != NUM_VOXELS && iz != NUM_VOXELS )
                gs_dyn_array_push(chunk->dirty_list, gs_v3(ix, iy, iz));
        }
    }
}

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
        uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);
        voxel_chunk_t chunk = {.origin = gs_vec3_scale(gs_v3(x, y, z), CHUNK_WORLD_SIZE)};
        gs_dyn_array_push(world.chunks, chunk);
        gs_hash_table_insert(world.chunk_map, gs_v3(x, y, z), idx);
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

    tick_scene();

    // For each chunk, build voxel data
    // const float scale = (float)CHUNK_WORLD_SIZE / (float)NUM_VOXELS;
    // static uint32_t idx = 0;
    // uint32_t cur_idx = idx; 
    // for (uint32_t i = 0; i < 1; ++i)
    // {
    //     idx = (idx + i) % TOTAL_CHUNKS;
    //     build_voxel_data(&chunks[idx]);
    // }

    // build_voxel_data(&chunks[0]);

    // for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
    //     for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
    //         for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    // {
    //     uint32_t _x = x + CHUNK_ARR_HALF_DIM;
    //     uint32_t _y = y + CHUNK_ARR_HALF_DIM;
    //     uint32_t _z = z + CHUNK_ARR_HALF_DIM;
    //     uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);
    //     build_voxel_data(&world.chunks[idx]);
    // }

    // static int cidx = 0;
    // if (gs_platform_key_pressed(GS_KEYCODE_C)) {
    //     cidx = (cidx + 1) % TOTAL_CHUNKS;
    // }

    // build_voxel_data(&chunks[cidx]);

    build_scene(&world);

    /* Immediate draw scene */
    gsi_camera(&gsi, &fps.cam);
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, false);

    // Draw ground plane
    // gsi_box(&gsi, 0.f, -0.1f * 0.5f, 0.f, 32.f, 0.1f, 32.f, 100, 100, 100, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Just draw small boxes at each corner representing occupancy values
    const float scale = (float)CHUNK_WORLD_SIZE / (float)NUM_VOXELS;

    // for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
    //     for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
    //         for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    for (uint32_t i = 0; i < gs_dyn_array_size(world.chunks); ++i)
    {
        // uint32_t _x = x + CHUNK_ARR_HALF_DIM;
        // uint32_t _y = y + CHUNK_ARR_HALF_DIM;
        // uint32_t _z = z + CHUNK_ARR_HALF_DIM;
        // uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);
        // voxel_chunk_t* chunk = &world.chunks[idx];
        voxel_chunk_t* chunk = &world.chunks[i];
        if (!chunk->is_dirty) continue;

        for (uint32_t ix = 0; ix < NUM_VOXELS; ++ix)
            for (uint32_t iy = 0; iy < NUM_VOXELS; ++iy)
                for (uint32_t iz = 0; iz < NUM_VOXELS; ++iz)
        {
        // }
        // for (uint32_t i = 0; i < gs_dyn_array_size(chunk->dirty_list); ++i) {

            // gs_vec3* _p = &chunk->dirty_list[i];

            // Calculate position of voxel using resolution scale and chunk origin   
            // float _x = (float)_p->x * scale;
            // float _y = (float)_p->y * scale;
            // float _z = (float)_p->z * scale;
            float _x = (float)ix * scale;
            float _y = (float)iy * scale;
            float _z = (float)iz * scale;
            gs_vec3 p = gs_vec3_add(chunk->origin, gs_v3(_x, _y, _z));

            // Change "redness" based on occupancy value
            gs_color_t col = GS_COLOR_RED;
            col.a = 255 - chunk->data[get_voxel_idx(ix, iy, iz)];

            // Draw box
            float d = 0.1f;
            float hd = d * 0.5f;
            if (col.a) {
                gsi_box(&gsi, p.x + hd, p.y + hd, p.z + hd, hd, hd, hd, col.r, col.g, col.b, col.a, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
            }
        }
    }

    // Draw them shits (get away from immediate drawing, it's too slow now to march cubes every frame).
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
        gs_vec3 ch = gs_vec3_add(world.chunks[idx].origin, gs_v3s(cwsh));
        gsi_box(&gsi, ch.x, ch.y, ch.z, cwsh, cwsh, cwsh, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    }

    // Draw 3d lines for axis at origin
    gs_vec3 origin = gs_v3s(0.f);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_XAXIS, 5.f), GS_COLOR_RED);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_YAXIS, 5.f), GS_COLOR_GREEN);
    gsi_line3Dv(&gsi, origin, gs_vec3_scale(GS_ZAXIS, 5.f), GS_COLOR_BLUE);

    gsi_defaults(&gsi);
    gsi_camera2D(&gsi);
    gs_snprintfc(txt, 256, "frame: %.2f", gs_engine_subsystem(platform)->time.frame);
    gsi_text(&gsi, 10.f, 30.f, txt, NULL, false, 255, 255, 255, 255);
    // gs_snprintf(txt, 256, "cidx: %zu", cidx);
    // gsi_text(&gsi, 10.f, 45.f, txt, NULL, false, 255, 255, 255, 255);

    /* Render */
    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

float op_subtract(float d1, float d2) {
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

#define NUM_SPHERES 10
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
        sphere_pos[i] = gs_vec3_scale(gs_vec3_add(gs_vec3_scale(gs_v3(x, y, z), 0.5f), gs_v3s(0.5f)), CHUNK_WORLD_SIZE * 2);
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

    // float b0 = sdf_box(gs_vec3_sub(gs_v3(8.f, 5.f, 4.f), p), gs_v3(5.f, 5.f, 1.f));
    float t0 = sdf_torus(gs_mat4_mul_vec3(tpmat, gs_vec3_sub(tpos, p)), gs_v2(5.f, 1.f));
    // float b1 = sdf_box(gs_vec3_sub(gs_v3(10.f, 5.f, 2.f), p), gs_v3(10.f, 5.f, 2.f));

    // const float t = gs_platform_elapsed_time();
    // dist = op_union(dist, b0); 
    dist = op_smooth_union(dist, t0, 1.f); 
    // dist = op_union(dist, b1);

    // Need a way to determine which chunks need to be updated.
    // Need a way to deal with boundaries across chunks.

    return dist;
}

typedef enum csg_op_type {
    UNION,
    INTERSECT, 
    SUBTRACT
} csg_op_type;

// Can give op type here, I suppose?

void place_sphere(voxel_world_t* world, csg_op_type op, gs_vec3 c, float r)
{
    // Find chunks that this sphere operates on (have some way to get chunks based on bounding region)
    aabb_t aabb = {0};
    gs_vec3 extents = gs_v3s(r);
    aabb.min = gs_vec3_sub(c, extents);
    aabb.max = gs_vec3_add(c, extents);

    const float scale = (float)CHUNK_WORLD_SIZE / (float)NUM_VOXELS;

    // Can't do it this way. Need an integral sized way of iterating.
    for (float x = aabb.min.x; x <= aabb.max.x; x += 1.f)
        for (float y = aabb.min.y; y <= aabb.max.y; y += 1.f)
            for (float z = aabb.min.z; z <= aabb.max.z; z += 1.f)
    {
        // Grab chunk at this position
        int32_t idx = get_chunk_idx(world, gs_v3(x, y, z));
        if (idx == -1) continue;

        voxel_chunk_t* chunk = &world->chunks[idx];

        // Find region to iterate that actually intersects with current chunk

        for (uint32_t ix = 0; ix < NUM_CORNERS; ++ix)
            for (uint32_t iy = 0; iy < NUM_CORNERS; ++iy)
                for (uint32_t iz = 0; iz < NUM_CORNERS; ++iz)
        {
            // Calculate position of voxel using resolution scale and chunk origin
            gs_vec3 p = gs_vec3_add(chunk->origin, gs_v3(ix * scale, iy * scale, iz * scale));

            // Evaluate sphere sdf here
            float v = sdf_sphere(gs_vec3_sub(c, p), r);

            if (v <= 0.5f) {

                uint8_t* d = &chunk->data[get_voxel_idx(ix, iy, iz)];
                float qv = (float)((float)*d / 255.f);

                // Get previous data, do op on it depending on what was selected
                switch (op) {
                    default:
                    case UNION:      qv = op_union(qv, v); break;
                    case INTERSECT : qv = op_intersection(qv, v); break;
                    case SUBTRACT:   qv = op_subtract(qv, v); break;
                }

                // Set data
                *d = quantize_float(qv, 0.f, 1.f);

                // chunk->data[get_voxel_idx(ix, iy, iz)] = quantize_float(v, 0.f, 1.f);
                // if (ix != NUM_VOXELS && iy != NUM_VOXELS && iz != NUM_VOXELS )
                    // gs_dyn_array_push(chunk->dirty_list, gs_v3(ix, iy, iz));
                chunk->is_dirty = true;
            }
        }
    }
}

// Want to be able to place a sphere into the world at some position + radius
void build_scene(voxel_world_t* world)
{
    // Clear dirty chunks
    // gs_dyn_array_clear(world->dirty_chunks);
    // Set all chunks to not dirty
    for (uint32_t i = 0; i < gs_dyn_array_size(world->chunks); ++i) {
        world->chunks[i].is_dirty = false;
        gs_dyn_array_clear(world->chunks[i].dirty_list);
        memset(world->chunks[i].data, 255, NUM_CORNERS * NUM_CORNERS * NUM_CORNERS);
    }

    // Given a position, need to grab a chunk
    // Also need to know the bounding area for the object being placed, so that I know which chunks are "dirty"
    // Let's build a simple sphere, radius 0.8f

    for (uint32_t i = 0; i < 1; ++i) {
        const float t = gs_platform_elapsed_time();
        const float st = sin(t * 0.001f * (i + 1));
        const float ct = cos(t * 0.002f * (i + 1));
        const gs_vec3 p = gs_vec3_add(gs_v3(8.f, 8.f, 8.f), gs_vec3_scale(gs_v3(st, ct, st * 2.f), CHUNK_WORLD_SIZE * 0.5f)); 
        const float r = (st * 0.5f + 0.5f) * 10.f + 1.f;
        place_sphere(world, UNION, p, r);
    }
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

// Instead, need to build vertex lists, then build triangle lists and normals from that list.
// Sampling normals in the other method via gradients is way too expensive (only want to evaluate the scene once per voxel).

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

uint8_t quantize_float(float v, float min, float max)
{
    return (uint8_t)gs_clamp((((v - min) / (max - min)) * 255.f), 0, 255);
}

void march_single_cube(gs_immediate_draw_t* gsi, voxel_chunk_t* chunk, gs_vec3 p, float scale)
{
    int32_t iCorner, iVertex, iVertexTest, iEdge, iTriangle, iFlagIndex, iEdgeFlags;
    float fOffset;
    gs_color_t sColor;
    float afCubeValue[8];
    gs_vec3 asEdgeVertex[12];
    gs_vec3 asEdgeNorm[12];
    gs_vec3 sp = gs_vec3_add(chunk->origin, gs_vec3_scale(p, scale));

    // Get values of cube corners
    for (uint32_t i = 0; i < 8; ++i) {
        gs_vec3 _p = gs_v3(
            p.x + a2fVertexOffset[i][0],
            p.y + a2fVertexOffset[i][1],
            p.z + a2fVertexOffset[i][2]
        );
        afCubeValue[i] = (float)(chunk->data[get_voxel_idx(_p.x, _p.y, _p.z)]) / 255.f;
    }

    //Find which vertices are inside of the surface and which are outside
    const float fTargetValue = 0.5f; // Still not sure about this, but for some reason it works?

    iFlagIndex = 0;
    for(iVertexTest = 0; iVertexTest < 8; iVertexTest++) {
        if(afCubeValue[iVertexTest] <= fTargetValue) 
                iFlagIndex |= 1<<iVertexTest;
    }

    if (iFlagIndex == 0 || iFlagIndex == 255) 
        return;


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

            asEdgeVertex[iEdge].x = sp.x + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][0] + fOffset * a2fEdgeDirection[iEdge][0]) * scale;
            asEdgeVertex[iEdge].y = sp.y + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][1] + fOffset * a2fEdgeDirection[iEdge][1]) * scale;
            asEdgeVertex[iEdge].z = sp.z + (a2fVertexOffset[ a2iEdgeConnection[iEdge][0]][2] + fOffset * a2fEdgeDirection[iEdge][2]) * scale;

            // asEdgeNorm[iEdge] = get_normal(asEdgeVertex[iEdge].x, asEdgeVertex[iEdge].y, asEdgeVertex[iEdge].z);
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
    const float scale = (float)CHUNK_WORLD_SIZE / (float)NUM_VOXELS;

    /*
    // For each chunk 
    for (int32_t x = -CHUNK_ARR_HALF_DIM; x <= CHUNK_ARR_HALF_DIM; ++x)
        for (int32_t y = -CHUNK_ARR_HALF_DIM; y <= CHUNK_ARR_HALF_DIM; ++y)
            for (int32_t z = -CHUNK_ARR_HALF_DIM; z <= CHUNK_ARR_HALF_DIM; ++z)
    {
        uint32_t _x = x + CHUNK_ARR_HALF_DIM;
        uint32_t _y = y + CHUNK_ARR_HALF_DIM;
        uint32_t _z = z + CHUNK_ARR_HALF_DIM;
        uint32_t idx = _x + CHUNK_ARR_DIM * (_y + CHUNK_ARR_DIM * _z);

        // If you could only march 'dirty' voxels, then this would be easier, right?
        // For each voxel
        // for (uint32_t ix = 0; ix < NUM_VOXELS; ++ix)
        //     for (uint32_t iy = 0; iy < NUM_VOXELS; ++iy)
        //         for (uint32_t iz = 0; iz < NUM_VOXELS; ++iz)
        // {
        //     // Calculate position of voxel using resolution scale and chunk origin   
        //     // gs_vec3 p = gs_vec3_add(chunks[idx].origin, gs_v3(ix * scale, iy * scale, iz * scale));
        //     gs_vec3 p = gs_v3(ix, iy, iz);

        //     // Then march single cube evaluating whatever scalar field you want to use.
        //     march_single_cube(gsi, &chunks[idx], p, scale);
        // }

        // gs_println("dirty: %zu", gs_dyn_array_size(chunks[idx].dirty_list));
        // Iterate dirty list
        for (uint32_t i = 0; i < gs_dyn_array_size(world.chunks[idx].dirty_list); ++i) {
            // Then march single cube evaluating whatever scalar field you want to use.
            march_single_cube(gsi, &world.chunks[idx], world.chunks[idx].dirty_list[i], scale);
        }
    }
    */

    // For each chunk in dirty list
    uint32_t dc = 0;
    for (uint32_t c = 0; c < gs_dyn_array_size(world.chunks); ++c)
    {
        voxel_chunk_t* cp = &world.chunks[c];

        if (cp->is_dirty) {
            dc++;

            for (uint32_t ix = 0; ix < NUM_VOXELS; ++ix)
                for (uint32_t iy = 0; iy < NUM_VOXELS; ++iy)
                    for (uint32_t iz = 0; iz < NUM_VOXELS; ++iz) {

                gs_vec3 p = gs_v3(ix, iy, iz);
                march_single_cube(gsi, &cp, p, scale);
            }

            // for (uint32_t v = 0; v < gs_dyn_array_size(cp->dirty_list); ++v) {
            //     march_single_cube(gsi, &cp, cp->dirty_list[v], scale);
            // }
        }
    }
    gs_println("dc: %zu", dc);
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
        .window_width = 800,
        .window_height = 600
    };
}   






