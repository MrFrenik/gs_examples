/*================================================================
    * Copyright: 2020 John Jackson
    * frame_buffer

    The purpose of this example is to demonstrate how to create a 
    frame buffer for off-screen rendering.
    
    * Render spinningline cube onto an offscreen buffer, then use render  
        target as texture for spinning 3D cube

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

gs_command_buffer_t                  gcb  = {0};
gs_immediate_draw_t                  gsi  = {0};
gs_handle(gs_graphics_render_pass_t) rp   = {0};
gs_handle(gs_graphics_framebuffer_t) fbo  = {0};
gs_handle(gs_graphics_texture_t)     rt   = {0};

void app_init()
{
    // Construct new command buffer
    gcb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();

    // Construct frame buffer
    fbo = gs_graphics_framebuffer_create(NULL);

    // Construct color render target
    rt = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t) {
            .width = gs_platform_framebuffer_width(gs_platform_main_window()),   // Width of texture in pixels
            .height = gs_platform_framebuffer_height(gs_platform_main_window()), // Height of texture in pixels
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,                          // Format of texture data (rgba32, rgba8, rgba32f, r8, depth32f, etc...)
            .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,                           // Wrapping type for s axis of texture
            .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,                           // Wrapping type for t axis of texture
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR,                     // Minification filter for texture
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR,                     // Magnification filter for texture
            .render_target = true
        }
    );

    // Construct render pass for offscreen render pass
    rp = gs_graphics_render_pass_create(
        &(gs_graphics_render_pass_desc_t){
            .fbo = fbo,                      // Frame buffer to bind for render pass
            .color = &rt,                    // Color buffer array to bind to frame buffer    
            .color_size = sizeof(rt)         // Size of color attachment array in bytes
        }
    );
 }

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Render pass action for clearing the screen
    gs_graphics_clear_desc_t fb_clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.0f, 0.0f, 0.0f, 1.f}};
    gs_graphics_clear_desc_t bb_clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 1.f}};

    // Immediate rendering for offscreen buffer
    gsi_camera3D(&gsi);
    gsi_transf(&gsi, 0.f, 0.f, -2.f);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0001f, GS_YAXIS);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0002f, GS_XAXIS);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0005f, GS_ZAXIS);
    gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 200, 100, 50, 255, GS_GRAPHICS_PRIMITIVE_LINES);

    // Bind render pass for offscreen rendering then draw to buffer
    gs_graphics_begin_render_pass(&gcb, rp);
        gs_graphics_set_viewport(&gcb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&gcb, &fb_clear);
        gsi_draw(&gsi, &gcb);
    gs_graphics_end_render_pass(&gcb);

    // Immediate rendering for back buffer
    gsi_camera3D(&gsi);
    gsi_depth_enabled(&gsi, true);
    gsi_face_cull_enabled(&gsi, true);
    gsi_transf(&gsi, 0.f, 0.f, -1.f);
    gsi_texture(&gsi, rt);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0001f, GS_YAXIS);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0002f, GS_XAXIS);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0003f, GS_ZAXIS);
    gsi_box(&gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Render to back buffer
    gs_graphics_begin_render_pass(&gcb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&gcb, 0, 0, (int32_t)fbs.x, (int32_t)fbs.y);
        gs_graphics_clear(&gcb, &bb_clear);
        gsi_draw(&gsi, &gcb);
    gs_graphics_end_render_pass(&gcb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update
    };
}   






