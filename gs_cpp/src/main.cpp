/*================================================================
    * Copyright: 2020 John Jackson
    * HelloGS_CPP

    A Bare bones application for getting started using `gunslinger`.
    Creates an appplication context, an engine context, and then 
    opens a main window for you using the rendering context in cpp.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

void update()
{
   if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    gs_app_desc_t desc = {};
    desc.update = update;
    return desc;
}   