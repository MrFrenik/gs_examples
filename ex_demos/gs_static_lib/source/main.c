/*================================================================
    * Copyright: 2020 John Jackson
    * gs_static_lib

    This example compiles gunslinger as a static library then 
    links it against the example program to run. 

    Press `esc` to exit the application.
================================================================*/

#include <gs/gs.h>

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .update = app_update
    };
}
