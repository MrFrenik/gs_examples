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

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    if (gs_platform_key_pressed(GS_KEYCODE_LEFT_SHIFT)) {
        gs_println("YES!");
    }

    if (gs_platform_mouse_down(GS_MOUSE_LBUTTON)) {
        gs_println("mouse lbutton down");
    }

    if (gs_platform_mouse_released(GS_MOUSE_LBUTTON)) {
        gs_println("mouse lbutton released");
    }
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .update = update
    };
}   