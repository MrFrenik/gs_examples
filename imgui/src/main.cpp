/*================================================================
    * Copyright: 2020 John Jackson
    * gs_imgui

    The purpose of this example is to show how to hook up 
    imgui with gunslinger as its backene.

    Included: 
        * imgui example

    Press `esc` to exit the application.
================================================================*/

#include "gs_imgui_incl.h"

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit(); 
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    gs_app_desc_t desc = {0};
    desc.update = app_update;
    return desc;
}