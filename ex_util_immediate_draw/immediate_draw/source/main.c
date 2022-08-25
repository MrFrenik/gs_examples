/*================================================================
    * Copyright: 2020 John Jackson
    * immediate_draw

    The purpose of this example is to demonstrate how to use the optional 
    immediate drawing utility, mainly to be used for quick debug rendering.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

gs_command_buffer_t  cb   = {0};
gs_immediate_draw_t  gsi  = {0};
gs_asset_font_t      font = {0};
gs_asset_texture_t   tex  = {0};
gs_texture           sdf_tex = {0};

void init()
{
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new(gs_platform_main_window());

	const char* asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
	char PATH[256] = {0};

	gs_snprintf(PATH, sizeof(PATH), "%s/font.ttf", asset_dir);
    gs_asset_font_load_from_file(PATH, &font, 48);

	gs_snprintf(PATH, sizeof(PATH), "%s/champ.png", asset_dir);
    gs_graphics_texture_desc_t desc = {0};
    gs_asset_texture_load_from_file(PATH, &tex, NULL, true, false);
}

void update()
{
    gs_vec2 ws = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Set up 2D camera for projection matrix
    gsi_camera2D(&gsi, (uint32_t)ws.x, (uint32_t)ws.y);

    // Rect
    gsi_rectv(&gsi, gs_v2(500.f, 50.f), gs_v2(600.f, 100.f), GS_COLOR_RED, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_rectv(&gsi, gs_v2(650.f, 50.f), gs_v2(750.f, 100.f), GS_COLOR_RED, GS_GRAPHICS_PRIMITIVE_LINES);

    // Triangle
    gsi_trianglev(&gsi, gs_v2(50.f, 50.f), gs_v2(100.f, 100.f), gs_v2(50.f, 100.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_trianglev(&gsi, gs_v2(200.f, 50.f), gs_v2(300.f, 100.f), gs_v2(200.f, 100.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_LINES);

    // Lines
    gsi_linev(&gsi, gs_v2(50.f, 20.f), gs_v2(500.f, 20.f), gs_color(0, 255, 0, 255));

    // Circle
    gsi_circle(&gsi, 350.f, 170.f, 50.f, 20, 100, 150, 220, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_circle(&gsi, 250.f, 170.f, 50.f, 20, 100, 150, 220, 255, GS_GRAPHICS_PRIMITIVE_LINES);

    // Circle Sector
    gsi_circle_sector(&gsi, 50.f, 150.f, 50.f, 0, 90, 32, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_circle_sector(&gsi, 150.f, 200.f, 50.f, 90, 270, 32, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);

    // Box (with texture)
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, true);
    gsi_camera3D(&gsi, (uint32_t)ws.x, (uint32_t)ws.y);
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_translatef(&gsi, -2.f, -1.f, -5.f);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.001f, GS_YAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0005f, GS_ZAXIS);
        gsi_texture(&gsi, tex.hndl);
        gsi_scalef(&gsi, 1.5f, 1.5f, 1.5f);
        gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
        gsi_texture(&gsi, (gs_handle(gs_graphics_texture_t)){0});
    }
    gsi_pop_matrix(&gsi);

    // Box (lines, no texture)
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_translatef(&gsi, 2.f, -1.f, -5.f);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.001f, GS_YAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0008f, GS_ZAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0009f, GS_XAXIS);
        gsi_scalef(&gsi, 1.5f, 1.5f, 1.5f);
        gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 200, 100, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    }
    gsi_pop_matrix(&gsi);
    
    // Sphere (triangles, no texture)
    gsi_camera3D(&gsi, (uint32_t)ws.x, (uint32_t)ws.y);
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_translatef(&gsi, -2.f, -1.f, -5.f);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.001f, GS_YAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0005f, GS_ZAXIS);
        gsi_scalef(&gsi, 1.5f, 1.5f, 1.5f);
        gsi_sphere(&gsi, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
    gsi_pop_matrix(&gsi);

    // Sphere (lines)
    gsi_push_matrix(&gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_translatef(&gsi, 2.f, -1.f, -5.f);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.001f, GS_YAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0008f, GS_ZAXIS);
        gsi_rotatev(&gsi, gs_platform_elapsed_time() * 0.0009f, GS_XAXIS);
        gsi_scalef(&gsi, 1.5f, 1.5f, 1.5f);
        gsi_sphere(&gsi, 0.f, 0.f, 0.f, 1.0f, 255, 255, 255, 50, GS_GRAPHICS_PRIMITIVE_LINES);
    }
    gsi_pop_matrix(&gsi);

    // Text (custom and default fonts)
    gsi_camera2D(&gsi, (uint32_t)ws.x, (uint32_t)ws.y);
    gsi_defaults(&gsi);
    gsi_text(&gsi, 410.f, 150.f, "Custom Font", &font, false, 200, 100, 50, 255);
    gsi_text(&gsi, 450.f, 200.f, "Default Font", NULL, false, 50, 100, 255, 255);

    gs_graphics_texture_desc_t q = {0};
    gs_graphics_texture_desc_query(sdf_tex, &q);
    gsi_texture(&gsi, sdf_tex);
    gsi_rectvd(&gsi, gs_v2s(100.f), gs_vec2_scale(gs_v2((float)q.width, (float)q.height), 3.f), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES); 

    gsi_renderpass_submit(&gsi, &cb, (uint32_t)ws.x, (uint32_t)ws.y, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






