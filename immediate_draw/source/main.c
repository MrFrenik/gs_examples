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

gs_command_buffer_t               cb  = {0};
gs_immediate_draw_t               gsi = {0};

void init()
{
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new();
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gsi_camera3D(&gsi);
    gsi_transf(&gsi, 0.f, 0.f, -2.f);
    gsi_rotatefv(&gsi, gs_platform_elapsed_time() * 0.0001f, GS_YAXIS);
    gsi_sphere(&gsi, 0.f, 0.f, 0.f, 1.f, 20, 50, 150, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_face_cull_enabled(&gsi, false);
    gsi_defaults(&gsi);
    gsi_scalef(&gsi, 0.02f, 0.02f, 0.02f);
    gsi_text(&gsi, 0.f, 0.f, "Sample Text", NULL, true, 255, 255, 255, 255);
    gsi_render_pass_submit(&gsi, &cb, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






