/*================================================================
    * Copyright: 2020 John Jackson
    * mouse

    A simple application to show to how query for mouse input
    and coordinates.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

typedef struct 
{
    gs_command_buffer_t cb;
    gs_gui_context_t gui;
} app_t;

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new(); 
    app->gui = gs_gui_new(gs_platform_main_window());
}

void app_update()
{
    app_t* app = gs_user_data(app_t);
    gs_gui_context_t* gui = &app->gui;
    gs_command_buffer_t* cb = &app->cb;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    bool mouse_down[3] = {0};
    bool mouse_pressed[3] = {0};
    bool mouse_released[3] = {0};

    // Quit application
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Query mouse held down states.
    mouse_down[GS_MOUSE_LBUTTON] = gs_platform_mouse_down(GS_MOUSE_LBUTTON);
    mouse_down[GS_MOUSE_RBUTTON] = gs_platform_mouse_down(GS_MOUSE_RBUTTON);
    mouse_down[GS_MOUSE_MBUTTON] = gs_platform_mouse_down(GS_MOUSE_MBUTTON);

    // Query mouse release states.
    mouse_released[GS_MOUSE_LBUTTON] = gs_platform_mouse_released(GS_MOUSE_LBUTTON);
    mouse_released[GS_MOUSE_RBUTTON] = gs_platform_mouse_released(GS_MOUSE_RBUTTON);
    mouse_released[GS_MOUSE_MBUTTON] = gs_platform_mouse_released(GS_MOUSE_MBUTTON);

    // Query mouse pressed states. Press is a single frame click. 
    mouse_pressed[GS_MOUSE_LBUTTON] = gs_platform_mouse_pressed(GS_MOUSE_LBUTTON);
    mouse_pressed[GS_MOUSE_RBUTTON] = gs_platform_mouse_pressed(GS_MOUSE_RBUTTON);
    mouse_pressed[GS_MOUSE_MBUTTON] = gs_platform_mouse_pressed(GS_MOUSE_MBUTTON);

    // Query for mouse position
    gs_vec2 mp = gs_platform_mouse_positionv();

    // Query for mouse wheel
    gs_vec2 mw = gs_platform_mouse_wheelv(); 

    // Query for mouse delta
    gs_vec2 md = gs_platform_mouse_deltav();

    // Query for mouse lock
    bool lock = gs_platform_mouse_locked();
    bool moved = gs_platform_mouse_moved();

    // Simple gui to display all of this information
    gs_gui_begin(gui, &(gs_gui_hints_t){.framebuffer_size = fbs, .viewport = {0.f, 0.f, fbs.x, fbs.y}});
    {
        const gs_vec2 ws = gs_v2(600.f, 300.f);
        gs_gui_window_begin(gui, "Mouse", gs_gui_rect((fbs.x - ws.x) * 0.5f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
        {
            #define GUI_LABEL(STR, ...)\
                do {\
                    gs_snprintfc(BUFFER, 256, STR, ##__VA_ARGS__);\
                    gs_gui_label(gui, BUFFER);\
                } while (0) 

            gs_gui_layout_row(gui, 1, (int32_t[]){-1}, 0); 

            GUI_LABEL("Position: <%.2f %.2f>", mp.x, mp.y);
            GUI_LABEL("Wheel: <%.2f %.2f>", mw.x, mw.y);
            GUI_LABEL("Delta: <%.2f %.2f>", md.x, md.y);
            GUI_LABEL("Lock: %zu", lock);
            GUI_LABEL("Moved: %zu", moved);

            struct {const char* str; int32_t val;} btns[] = {
                {"Left", GS_MOUSE_LBUTTON}, 
                {"Right", GS_MOUSE_RBUTTON}, 
                {"Middle", GS_MOUSE_MBUTTON},
                {NULL}
            };

            gs_gui_layout_row(gui, 7, (int32_t[]){100, 100, 32, 100, 32, 100, 32}, 0);
            for (uint32_t i = 0; btns[i].str; ++i)
            { 
                GUI_LABEL("%s: ", btns[i].str);
                GUI_LABEL("pressed: "); GUI_LABEL("%d", mouse_pressed[btns[i].val]);
                GUI_LABEL("down: "); GUI_LABEL("%d", mouse_down[btns[i].val]);
                GUI_LABEL("released: "); GUI_LABEL("%d", mouse_released[btns[i].val]);
            }
        }
        gs_gui_window_end(gui);
    }
    gs_gui_end(gui);

    // Render gui pass
    gs_gui_renderpass_submit(gui, cb, (gs_color_t){10, 10, 10, 255}); 

    // Submit to graphics backend
    gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{ 
    // Want to still be able to use this syntax...
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_free(&app->cb);
    gs_gui_free(&app->gui);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown
    };
}   
