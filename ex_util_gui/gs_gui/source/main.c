/*================================================================
    * Copyright: 2020 John Jackson
    * gs_gui

    The purpose of this example is to demonstrate how to use the gs_gui
    util.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

gs_command_buffer_t gcb = {0};
gs_gui_context_t gsgui = {0}; 

//Color picker
static float bg[3] = {90, 95, 100};

// Widgets/Windows
void write_log(const char* text);
void log_window(gs_gui_context_t* ctx);
void test_window(gs_gui_context_t* ctx);
void dockspace(gs_gui_context_t* ctx);

void app_init()
{
    gcb = gs_command_buffer_new();
    gs_gui_init(&gsgui, gs_platform_main_window());

    // Dock windows before hand
    gs_gui_dock_ex(&gsgui, "Style_Editor", "Demo_Window", GS_GUI_SPLIT_TAB, 0.5f);
} 

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
        gs_quit();
    }

    // Begin new frame for gui
    gs_gui_begin(&gsgui, NULL);

    dockspace(&gsgui);
    gs_gui_demo_window(&gsgui, gs_gui_rect(100, 100, 500, 500), NULL);
    gs_gui_style_editor(&gsgui, NULL, gs_gui_rect(350, 250, 300, 240), NULL);

    // End gui frame
    gs_gui_end(&gsgui);

    // Do rendering
    gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {bg[0]/255, bg[1]/255,bg[2]/255, 1.f}}};
    gs_graphics_renderpass_begin(&gcb, (gs_handle(gs_graphics_renderpass_t)){0});
    {
        gs_graphics_clear(&gcb, &clear);
        gs_graphics_set_viewport(&gcb,0,0,(int)fbs.x,(int)fbs.y);
        gs_gui_render(&gsgui, &gcb);
    }
    gs_graphics_renderpass_end(&gcb);
    
    //Submits to cb
    gs_graphics_command_buffer_submit(&gcb);
}

void app_shutdown()
{
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window.width = 1024,
        .window.height = 760
    };
}

void dockspace(gs_gui_context_t* ctx) 
{
    int32_t opt = GS_GUI_OPT_NOCLIP | GS_GUI_OPT_NOFRAME | GS_GUI_OPT_FORCESETRECT | GS_GUI_OPT_NOTITLE | GS_GUI_OPT_DOCKSPACE | GS_GUI_OPT_FULLSCREEN | GS_GUI_OPT_NOMOVE | GS_GUI_OPT_NOBRINGTOFRONT | GS_GUI_OPT_NOFOCUS | GS_GUI_OPT_NORESIZE;
    gs_gui_window_begin_ex(ctx, "Dockspace", gs_gui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Empty dockspace...
    }
    gs_gui_window_end(ctx);
} 

