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

enum {
    GUI_STYLE_ROOT = 0x00,
    GUI_STYLE_TITLE,
    GUI_STYLE_BUTTON,
    GUI_STYLE_COUNT
};

enum {
    GUI_FONT_LABEL = 0x00,
    GUI_FONT_BUTTON, 
    GUI_FONT_BUTTONFOCUS,
    GUI_FONT_COUNT
};

typedef struct
{
    gs_command_buffer_t cb;
    gs_gui_context_t    gui;
    gs_immediate_draw_t gsi;
    gs_gui_style_t styles[GUI_STYLE_COUNT][GS_GUI_ELEMENT_STATE_COUNT];
    gs_asset_font_t fonts[GUI_FONT_COUNT];
    gs_asset_texture_t logo;
    gs_asset_texture_t bg;
} app_t; 

void app_init()
{
    app_t* app = gs_engine_user_data(app_t);
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

    // Set up styles for elements 
    gs_gui_style_element_t panel_style[] = {
        {GS_GUI_STYLE_PADDING_TOP, 20},
        {GS_GUI_STYLE_BORDER_COLOR, .color = gs_color(0, 0, 0, 0)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(0, 0, 0, 0)}
    };

    gs_gui_style_element_t button_style[] = {
        {GS_GUI_STYLE_ALIGN_CONTENT, GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, GS_GUI_JUSTIFY_CENTER},
        {GS_GUI_STYLE_WIDTH, 200},
        {GS_GUI_STYLE_HEIGHT, 50},
        {GS_GUI_STYLE_FONT, &app->fonts[GUI_FONT_BUTTON]},
        {GS_GUI_STYLE_MARGIN_LEFT, 0},
        {GS_GUI_STYLE_MARGIN_TOP, 10}, 
        {GS_GUI_STYLE_MARGIN_BOTTOM, 0},
        {GS_GUI_STYLE_MARGIN_RIGHT, 20},
        {GS_GUI_STYLE_SHADOW_X, 1},
        {GS_GUI_STYLE_SHADOW_Y, 1}, 
        {GS_GUI_STYLE_SHADOW_COLOR, .color = gs_color(146, 146, 146, 200)},
        {GS_GUI_STYLE_BORDER_COLOR, .color = GS_COLOR_BLACK},
        {GS_GUI_STYLE_BORDER_WIDTH, 2},
        {GS_GUI_STYLE_CONTENT_COLOR, .color = gs_color(67, 67, 67, 255)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(198, 198, 198, 255)}
    };

    gs_gui_style_element_t button_hover_style[] = {
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(168, 168, 168, 255)}
    };

    gs_gui_style_element_t button_focus_style[] = {
        {GS_GUI_STYLE_FONT, &app->fonts[GUI_FONT_BUTTONFOCUS]},
        {GS_GUI_STYLE_CONTENT_COLOR, .color = gs_color(255, 255, 255, 255)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(49, 174, 31, 255)}
    }; 

    gs_gui_style_element_t label_style[] = {
        {GS_GUI_STYLE_FONT, &app->fonts[GUI_FONT_LABEL]},
        {GS_GUI_STYLE_ALIGN_CONTENT, GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, GS_GUI_JUSTIFY_END}
    }; 

    gs_gui_style_element_t text_style[] = {
        {GS_GUI_STYLE_FONT, &app->fonts[GUI_FONT_LABEL]},
        {GS_GUI_STYLE_ALIGN_CONTENT, GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, GS_GUI_JUSTIFY_START}
    }; 

    // Bind styles (NOTE(john): this will be set up as stylesheets in the future)
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_PANEL, -1, panel_style, sizeof(panel_style)); 
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_LABEL, -1, label_style, sizeof(label_style));
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_TEXT, -1, text_style, sizeof(text_style));
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_BUTTON, -1, button_style, sizeof(button_style));
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_BUTTON, GS_GUI_ELEMENT_STATE_HOVER, button_hover_style, sizeof(button_hover_style));
    gs_gui_set_element_style(&app->gui, GS_GUI_ELEMENT_BUTTON, GS_GUI_ELEMENT_STATE_FOCUS, button_focus_style, sizeof(button_focus_style));
} 

// Simple custom button command that allows us to render some inner highlights and shadows
bool gui_custom_button(gs_gui_context_t* ctx, const char* str)
{
    bool ret = gs_gui_button(ctx, str);
    gs_gui_rect_t rect = ctx->last_rect; 

    // Draw inner shadows/highlights over button
    gs_color_t hc = GS_COLOR_WHITE, sc = gs_color(85, 85, 85, 255);
    int32_t w = 2;
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + w, rect.y, rect.w - 2 * w, w), hc);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + w, rect.y + rect.h - w, rect.w - 2 * w, w), sc);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x, rect.y, w, rect.h), hc);
	gs_gui_draw_rect(ctx, gs_gui_rect(rect.x + rect.w - w, rect.y, w, rect.h), sc);

    return ret;
}

void app_update()
{
    // Cache data for frame
    app_t* app = gs_engine_user_data(app_t);
    gs_gui_context_t* gui = &app->gui;
    gs_command_buffer_t* cb = &app->cb;
    gs_immediate_draw_t* gsi = &app->gsi;
    gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const float _t = gs_platform_elapsed_time() * 0.0001f;

    // Query information about our textures for resolutions
    gs_graphics_texture_desc_t desc = {0};

    // Logo size
    gs_graphics_texture_desc_query(app->logo.hndl, &desc);
    const gs_vec2 logo_sz = gs_v2(desc.width, desc.height); 

    // Background size
    gs_graphics_texture_desc_query(app->bg.hndl, &desc); 
    const gs_vec2 bg_sz = gs_v2(desc.width, desc.height);

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) 
    {
        gs_engine_quit();
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
        if (gs_gui_begin_window_ex(
                gui, "#root", gs_gui_rect(0, 0, 0, 0), 
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
            gs_gui_layout_t* l = NULL; 

            // Logo
            {
                // Capture current layout
                l = gs_gui_get_layout(gui); 
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&l->body, logo_sz.x, logo_sz.y, 0, -200, GS_GUI_LAYOUT_ANCHOR_CENTER), 0);
                gs_gui_draw_image(gui, app->logo.hndl, gs_gui_layout_next(gui), gs_v2s(0.f), gs_v2s(1.f), GS_COLOR_WHITE);
            }

            // buttons panel
            const gs_vec2 menu_sz = gs_v2(logo_sz.x - 150.f, fbs.y); 
            gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&gui->last_rect, menu_sz.x, menu_sz.y, 0, 140, GS_GUI_LAYOUT_ANCHOR_TOPCENTER), 0); 
            gs_gui_begin_panel_ex(gui, "#buttons", GS_GUI_OPT_NOSCROLL);
            { 
                l = gs_gui_get_layout(gui); 

                // single player | multiplayer | realms
                gs_gui_layout_row(gui, 1, (int[]){-1}, 0); 
                gui_custom_button(gui, "Singleplayer");
                gui_custom_button(gui, "Multiplayer");
                gui_custom_button(gui, "Minecraft Realms");

                const float spacing = (float)gui->styles[GS_GUI_ELEMENT_BUTTON][0x00].margin[GS_GUI_MARGIN_RIGHT] * 0.5f;
                const float w = l->body.w * 0.5f - spacing;
                gs_gui_layout_row(gui, 2, (int[]){w, w}, 0); 
                gui_custom_button(gui, "Settings");
                gui_custom_button(gui, "Exit Game");

            }
            gs_gui_end_panel(gui); 

            // Version
            {
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 500, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMLEFT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_gui_draw_control_text(gui, "Version 0.69", next, GS_GUI_ELEMENT_TEXT, 0x00, 0x00); 
            }

            // Copyright
            {
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 500, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_gui_draw_control_text(gui, "Copyright Gunslinger. Please, do distrubute.", next, GS_GUI_ELEMENT_LABEL, 0x00, 0x00); 
            }

            // Frames
            {
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_TOPRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_snprintfc(TMP, 256, "frame: %.2f", gs_engine_subsystem(platform)->time.frame);
                gs_gui_draw_control_text(gui, TMP, next, GS_GUI_ELEMENT_LABEL, 0x00, 0x00); 
            }

            gs_gui_end_window(gui);
        }
    } 

    static bool enabled = false;
    if (gs_platform_key_pressed(GS_KEYCODE_F1)) enabled = !enabled; 

    // This is an issue...I want all of the default styles for an in-game debug window...
    if (enabled)
    {
        // Need to do something like this for that to work...
        // gs_gui_set_style_sheet(gui, &sheet);

        gs_gui_begin_window_ex(gui, "#debug", gs_gui_rect(0, 0, 300, 200), 0x00);
        {
            gs_gui_label(gui, "Blah");
        }
        gs_gui_end_window(gui);
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

void app_shutdown()
{
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window_width = 1024,
        .window_height = 760,
        .frame_rate = 60
    };
}


