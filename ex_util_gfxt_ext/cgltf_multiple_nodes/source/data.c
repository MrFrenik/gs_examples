#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif
/*================================================================
    * Copyright: 2020 John Jackson
================================================================*/
int32_t gui_opts = GS_GUI_OPT_NOTITLE |
        GS_GUI_OPT_NORESIZE |
        GS_GUI_OPT_NOMOVE |
        GS_GUI_OPT_FULLSCREEN |
        GS_GUI_OPT_FORCESETRECT |
        GS_GUI_OPT_NORESIZE |
        GS_GUI_OPT_NOFRAME |
        GS_GUI_OPT_NOTITLE;

typedef struct {
    gs_gui_context_t gui;
    gs_command_buffer_t cb;

    gs_gfxt_pipeline_t pip;
    gs_gfxt_material_t mat;
    gs_gfxt_mesh_t mesh;
    gs_gfxt_texture_t texture;

    const char* asset_dir;
    gs_camera_t camera;
    
    gs_gfxt_scene_t scene;
} app_t;

void ortho3(gs_vec3* left, gs_vec3* up, gs_vec3 v)
{
	*left = (v.z*v.z) < (v.x*v.x) ? gs_v3(v.y,-v.x,0) : gs_v3(0,-v.z,v.y);
	*up = gs_vec3_cross(*left, v);
}

void ray_cast(gs_aabb_t* aabb, gs_camera_t* camera, gs_vqs* xform, gs_contact_info_t* res, gs_vec2 fbs) {

    // Ray cast against various shapes in scene
    const float ray_len = 1000.f;
    const gs_vec2 mc = gs_platform_mouse_positionv(); // Mouse coordinate
    const gs_vec3 ms = gs_v3(mc.x, mc.y, 0.f);        // Mouse coordinate start with z = 0.f
    const gs_vec3 me = gs_v3(mc.x, mc.y, -ray_len);   // Mouse coordinate end with z = -1000.f (for ray cast into screen)
    const gs_vec3 ro = gs_camera_screen_to_world(camera, ms, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y);
    const gs_vec3 rd = gs_camera_screen_to_world(camera, me, 0, 0, (uint32_t)fbs.x, (uint32_t)fbs.y);

    gs_ray_t ray = {
        .p = ro,
        .d = gs_vec3_norm(gs_vec3_sub(ro, rd)),
        .len = ray_len
    };

    gs_aabb_vs_ray(aabb, xform, &ray, NULL, res);
}
