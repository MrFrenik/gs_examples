/*================================================================
    * Copyright: 2020 John Jackson
    * gs_gui style sheets example

    The purpose of this example is to demonstrate how to use the style sheets 
    for custom gui styling. This demo shows how to create them programmatically 
    as well as how to load from a resource data file.

    Press `esc` to exit the application.
================================================================*/

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
    const char* asset_dir;
    gs_asset_font_t font;
    gs_gui_style_sheet_t style_sheet;
} app_t; 

void gui_cb(gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd);
void reload_style_sheet();

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new(); 
    gs_gui_init(&app->gui, gs_platform_main_window()); 
    app->asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
    
    // Load in custom font file and then initialize gui font stash
    gs_snprintfc(FONT_PATH, 256, "%s/%s", app->asset_dir, "fonts/mc_regular.otf");
    gs_asset_font_load_from_file(FONT_PATH, &app->font, 20);
    gs_gui_init_font_stash(&app->gui, &(gs_gui_font_stash_desc_t){
        .fonts = (gs_gui_font_desc_t[]){{.key = "mc_regular", .font = &app->font}},
        .size = 1 * sizeof(gs_gui_font_desc_t)
    });

    // Load style sheet from file now
    reload_style_sheet();
} 

void app_update()
{
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_t* cb = &app->cb;
    gs_gui_context_t* gui = &app->gui;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const float t = gs_platform_elapsed_time();

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
        gs_quit();
    }

    // Begin new frame for gui
    gs_gui_begin(gui, fbs); 

    const gs_vec2 ws = gs_v2(500.f, 300.f);
    gs_gui_begin_window(gui, "Window", gs_gui_rect((fbs.x - ws.x) * 0.5f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
    {
        // Cache the current container 
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui); 

        gs_gui_button(gui, "Button"); 
        gs_gui_label(gui, "Label");
    } 
    gs_gui_end_window(gui);

    // End gui frame
    gs_gui_end(gui);

    // Do rendering
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05f, 0.05f, 1.f}}};
    gs_graphics_begin_render_pass(cb, (gs_handle(gs_graphics_render_pass_t)){0});
    {
        gs_graphics_clear(cb, &clear);
        gs_graphics_set_viewport(cb,0,0,(int)fbs.x,(int)fbs.y);
        gs_gui_render(gui, cb);
    }
    gs_graphics_end_render_pass(cb);
    
    //Submits to cb
    gs_graphics_submit_command_buffer(cb);
}

void app_shutdown()
{
    app_t* app = gs_user_data(app_t);
    gs_gui_free(&app->gui); 
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window_width = 1024,
        .window_height = 760
    };
}

void reload_style_sheet()
{
    app_t* app = gs_user_data(app_t);
    gs_gui_style_sheet_destroy(&app->gui, &app->style_sheet);
    gs_snprintfc(TMP, 256, "%s/%s", app->asset_dir, "style_sheets/gui.ss");
    app->style_sheet = gs_gui_style_sheet_load_from_file(&app->gui, TMP);
    gs_gui_set_style_sheet(&app->gui, &app->style_sheet);
}











