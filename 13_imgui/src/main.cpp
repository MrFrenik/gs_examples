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

gs_command_buffer_t gcb = {};
gs_imgui_t gsimgui = {};
bool show_demo_window = true;

void app_init()
{
	gcb = gs_command_buffer_new();
    gsimgui = gs_imgui_new(gs_platform_main_window(), false);
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gs_imgui_new_frame(&gsimgui);

	ImGui::ShowDemoWindow(&show_demo_window);

    gs_imgui_render(&gsimgui, &gcb);

    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    gs_app_desc_t desc = {0};
    desc.init = app_init;
    desc.update = app_update;
    return desc;
}