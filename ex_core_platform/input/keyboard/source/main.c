/*================================================================
    * Copyright: 2020 John Jackson
    * keyboard

    A simple application to show to how query for keyboard input.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

void update()
{
    // Quit application
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Key held down for more than a single frame
    if (gs_platform_key_down(GS_KEYCODE_A)) gs_println("A held");

    // Key released after either being pressed or held
    if (gs_platform_key_released(GS_KEYCODE_A)) gs_println("A released");

    // Key pressed and released once
    if (gs_platform_key_pressed(GS_KEYCODE_B)) gs_println("B pressed");

    // Grab input struct from gs platform, can use that directly
    gs_platform_input_t* input = &gs_subsystem(platform)->input;

    // Key down 
    if (input->key_map[GS_KEYCODE_Z] && input->prev_key_map[GS_KEYCODE_Z]) gs_println("Z held");

    // Single press
    if (input->key_map[GS_KEYCODE_C] && !input->prev_key_map[GS_KEYCODE_C]) gs_println("C pressed");

    // Release
    if (!input->key_map[GS_KEYCODE_Z] && input->prev_key_map[GS_KEYCODE_Z]) gs_println("Z released");
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .update = update
    };
}   
