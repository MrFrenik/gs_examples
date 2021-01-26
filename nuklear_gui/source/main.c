/*================================================================
    * Copyright: 2020 John Jackson
    * nuklear_gui

    The purpose of this example is to 

    Included: 
        *

    Press `esc` to exit the application.
================================================================*/

// Nuklear will define these, so we will not implement them ourselves
#include <gs/gs.h>
#include <Nuklear/nuklear.h>

gs_command_buffer_t gcb = {0};

void init()
{
    gcb = gs_command_buffer_new();
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Final command buffer submit
    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}