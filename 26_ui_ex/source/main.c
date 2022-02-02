/*================================================================
    * Copyright: 2020 John Jackson
    * simple custom ui example

    The purpose of this example is to demonstrate how to use the gs_gui
    util to make a simple 'game-like' ui.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_GUI_IMPL
#include <gs/util/gs_gui.h>

#include "data.c"

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new();
    app->gsi = gs_immediate_draw_new(gs_platform_main_window());
    gs_gui_init(&app->gui, gs_platform_main_window());

    // Load necessary assets
    gs_asset_font_load_from_file("./assets/mc_regular.otf", &app->fonts[GUI_FONT_LABEL], 24);
    gs_asset_font_load_from_file("./assets/mc_regular.otf", &app->fonts[GUI_FONT_BUTTON], 32); 
    gs_asset_font_load_from_file("./assets/mc_regular.otf", &app->fonts[GUI_FONT_BUTTONFOCUS], 34); 
    gs_asset_texture_load_from_file("./assets/mc.png", &app->logo, &(gs_graphics_texture_desc_t){
        .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
        .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
        .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST, 
        .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
        .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
    }, false, false);
    gs_asset_texture_load_from_file("./assets/mcbg.png", &app->bg, NULL, false, false); 

    // Generate new style sheet to use for menu
    app->menu_style_sheet = gs_gui_style_sheet_new(&app->gui, &menu_style_sheet_desc); 

    // Dock debug windows
    gs_gui_dock_ex(&app->gui, "Demo_Window", "Style_Editor", GS_GUI_SPLIT_TAB, 0.5f);
} 

void app_update()
{
    // Cache data for frame
    app_t* app = gs_user_data(app_t);
    gs_gui_context_t* gui = &app->gui;
    gs_command_buffer_t* cb = &app->cb;
    gs_immediate_draw_t* gsi = &app->gsi;
    gs_immediate_draw_t* odl = &gui->overlay_draw_list;
    gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const float _t = gs_platform_elapsed_time() * 0.0001f;
    const float dt = gs_platform_delta_time(); 
    static bool debug_enabled = false;

    // Query information about our textures for resolutions
    gs_graphics_texture_desc_t desc = {0};

    // Logo size
    gs_graphics_texture_desc_query(app->logo.hndl, &desc);
    const gs_vec2 logo_sz = gs_v2(desc.width, desc.height); 

    // Background size
    gs_graphics_texture_desc_query(app->bg.hndl, &desc); 
    const gs_vec2 bg_sz = gs_v2(desc.width, desc.height);

    // Overall menu size
    const gs_vec2 menu_sz = gs_v2(logo_sz.x, 500.f); 
    
    // Button panel size
    const gs_vec2 btn_panel_sz = gs_v2(menu_sz.x - 150.f, 320.f); 

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) 
    {
        gs_quit();
    } 

    // "Game" Scene (simple sin scrolling image to simulate a camera moving over a 3d scene)
    { 
        gs_vec2 os = gs_v2(sin(_t * 0.3f) * 200.f, sin(_t * 0.5f) * 20.f);
        gs_vec2 pos = gs_v2((fbs.x - bg_sz.x) * 0.5f + os.x, (fbs.y - bg_sz.y) * 0.5f + os.y);
        gsi_camera2D(gsi);
        gsi_texture(gsi, app->bg.hndl);
        gsi_rectvd(gsi, pos, bg_sz, gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES); 
    } 

    // Begin new frame for gui
    gs_gui_begin(gui); 
    { 
        // Set menu style sheet
        gs_gui_set_style_sheet(&app->gui, &app->menu_style_sheet);

        if (gs_gui_begin_window_ex(
                gui, "#root", gs_gui_rect(0, 0, 0, 0), NULL,
                    GS_GUI_OPT_NOFRAME | 
                    GS_GUI_OPT_NOTITLE | 
                    GS_GUI_OPT_NOMOVE | 
                    GS_GUI_OPT_FULLSCREEN | 
                    GS_GUI_OPT_NORESIZE | 
                    GS_GUI_OPT_NODOCK | 
                    GS_GUI_OPT_NOBRINGTOFRONT
        ))
        { 
            gs_gui_container_t* cnt = gs_gui_get_current_container(gui);
            gs_gui_layout_t* l = gs_gui_get_layout(gui); 

            // Inline style for button 
            gs_gui_layout_row(gui, 2, (int[]){0, 0}, 0);
            gs_gui_push_inline_style(gui, GS_GUI_ELEMENT_BUTTON, &btn_inline_style);
            gs_gui_button(gui, "Inline style"); 
            gs_gui_button(gui, "Inline style 2"); 
            gs_gui_pop_inline_style(gui, GS_GUI_ELEMENT_BUTTON);

            gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&l->body, menu_sz.x, menu_sz.y, 0, 0, GS_GUI_LAYOUT_ANCHOR_CENTER), 0);
            gs_gui_begin_panel_ex(gui, "#menu", GS_GUI_OPT_NOSCROLL);
            {
                // Logo
                {
                    // Capture current layout
                    l = gs_gui_get_layout(gui); 
                    gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&l->body, logo_sz.x, logo_sz.y, 0, 0, GS_GUI_LAYOUT_ANCHOR_TOPCENTER), 0);
                    gs_gui_draw_image(gui, app->logo.hndl, gs_gui_layout_next(gui), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_WHITE);
                }

                // buttons panel
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&l->body, btn_panel_sz.x, btn_panel_sz.y, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMCENTER), 0); 
                gs_gui_begin_panel_ex(gui, "#buttons", GS_GUI_OPT_NOSCROLL);
                { 
                    l = gs_gui_get_layout(gui); 

                    // single player | multiplayer | realms
                    gs_gui_layout_row(gui, 1, (int[]){-1}, 0);  // one item per row, set width to -1 to fill entire region, height to 0 to reference style sheet
                    button_custom(gui, "Singleplayer");
                    button_custom(gui, "Multiplayer");
                    if (button_custom(gui, "Options")) {debug_enabled = !debug_enabled;}
                    button_custom(gui, "Minecraft Realms");

                    const float spacing = (float)gui->style_sheet->styles[GS_GUI_ELEMENT_BUTTON][0x00].margin[GS_GUI_MARGIN_RIGHT] * 0.5f;
                    const float w = l->body.w * 0.5f - spacing;
                    gs_gui_layout_row(gui, 2, (int[]){w, w}, 0);
                    button_custom(gui, "Settings");
                    button_custom(gui, "Exit Game");
                }
                gs_gui_end_panel(gui); 
            }
            gs_gui_end_panel(gui); 

            // Version
            {
                const char* str = "Version 0.69";
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, 50, 10, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMLEFT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20)); gs_gui_layout_set_next(gui, next, 0);
                gs_gui_label(gui, str); 
            }

            // Copyright
            {
                const char* str = "Copyright Gunslinger. Please, do distrubute.";
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 500, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20)); gs_gui_layout_set_next(gui, next, 0);
                gs_gui_label(gui, str); 
            }

            // Frames
            {
                gs_snprintfc(TMP, 256, "frame: %.2f", gs_subsystem(platform)->time.frame);
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_TOPRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20)); gs_gui_layout_set_next(gui, next, 0);
                gs_gui_label(gui, TMP); 
            } 

            gs_gui_end_window(gui);
        } 

        if (gs_platform_key_pressed(GS_KEYCODE_F1)) debug_enabled = !debug_enabled; 

        // Set style sheet to default sheet
        gs_gui_set_style_sheet(gui, NULL); 
        gs_gui_demo_window(gui, gs_gui_rect(200, 100, 500, 250), &debug_enabled);
        gs_gui_style_editor(gui, &app->menu_style_sheet, gs_gui_rect(100, 100, 100, 100), &debug_enabled);
    } 

    // End gui frame
    gs_gui_end(gui); 

    // Do rendering
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {10.f/255.f, 10.f/255.f, 10.f/255.f, 1.f}}};
    gs_graphics_begin_render_pass(cb, (gs_handle(gs_graphics_render_pass_t)){0});
    {
        // Set view port
        gs_graphics_set_viewport(cb,0,0,(int)fbs.x,(int)fbs.y);

        // Clear screen
        gs_graphics_clear(cb, &clear); 

        // Render gsi
        gsi_render_pass_submit_ex(gsi, cb, NULL);

        // Render gui
        gs_gui_render(gui, cb);
    }
    gs_graphics_end_render_pass(cb);
    
    //Submits to cb
    gs_graphics_submit_command_buffer(cb);
}

int32_t button_custom(gs_gui_context_t* ctx, const char* label)
{ 
    // Do original button call
    int32_t res = gs_gui_button(ctx, label);
    
    // Draw inner shadows/highlights over button
    gs_color_t hc = GS_COLOR_WHITE, sc = gs_color(85, 85, 85, 255);
    gs_gui_rect_t r = ctx->last_rect;
    int32_t w = 2;
    gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w, r.y, r.w - 2 * w, w), hc);
    gs_gui_draw_rect(ctx, gs_gui_rect(r.x + w, r.y + r.h - w, r.w - 2 * w, w), sc);
    gs_gui_draw_rect(ctx, gs_gui_rect(r.x, r.y, w, r.h), hc);
    gs_gui_draw_rect(ctx, gs_gui_rect(r.x + r.w - w, r.y, w, r.h), sc);

    return res;
} 

void app_shutdown()
{
    // free gui
    app_t* app = gs_user_data(app_t);
    gs_gui_free(&app->gui);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .user_data = &g_app,
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window_width = 900,
        .window_height = 580
    };
}


