#define GS_PLATFORM_IMPL_DEFAULT
#define GS_PLATFORM_IMPL_CUSTOM
#define GS_IMPL
#include <gs/gs.h>

// Avoid gs platform layer? Could try that.
// See, how would you use gunslinger with a custom platform layer though? Without having to use GS's platform layer stuff.

gs_command_buffer_t cb = {0};

void app_init() 
{
    cb = gs_command_buffer_new();
}

void app_update() 
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    const float t = gs_platform_elapsed_time();
    // gs_println("t: %.2f", t);
    float r = ((sin(t * 0.001f) * 0.5f + 0.5f));
    float g = ((sin(t * 0.002f) * 0.5f + 0.5f));
    float b = ((sin(t * 0.003f) * 0.5f + 0.5f));

    // Try to clear screen?
    gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t){.actions = &(gs_graphics_clear_action_t){.color = {r, g, b, 1.f}}};
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
    gs_graphics_clear(&cb, &clear);
    gs_graphics_end_render_pass(&cb);
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .init = app_init,
        .update = app_update,
        .window_width = 500,
        .window_height = 400
    }; 
}
