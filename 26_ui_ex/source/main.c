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
    gs_asset_font_t fonts[GUI_FONT_COUNT];
    gs_asset_texture_t logo;
    gs_asset_texture_t bg;
    gs_gui_style_sheet_t menu_style_sheet;
} app_t; 

void style_window(gs_gui_context_t *ctx, gs_gui_style_sheet_t* style_sheet, bool* open); 

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
        {GS_GUI_STYLE_PADDING_TOP, .value = 20},
        {GS_GUI_STYLE_BORDER_COLOR, .color = gs_color(0, 0, 0, 0)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(0, 0, 0, 0)}
    };

    gs_gui_style_element_t button_style[] = {
        // type, value
        {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_CENTER},
        {GS_GUI_STYLE_WIDTH, .value = 200},
        {GS_GUI_STYLE_HEIGHT, .value = 45},
        {GS_GUI_STYLE_FONT, .font = &app->fonts[GUI_FONT_BUTTON]},
        {GS_GUI_STYLE_MARGIN_LEFT, .value = 0},
        {GS_GUI_STYLE_MARGIN_TOP, .value = 10}, 
        {GS_GUI_STYLE_MARGIN_BOTTOM, .value = 0},
        {GS_GUI_STYLE_MARGIN_RIGHT, .value = 20},
        {GS_GUI_STYLE_SHADOW_X, .value = 1},
        {GS_GUI_STYLE_SHADOW_Y, .value = 1}, 
        {GS_GUI_STYLE_SHADOW_COLOR, .color = gs_color(146, 146, 146, 200)},
        {GS_GUI_STYLE_BORDER_COLOR, .color = GS_COLOR_BLACK},
        {GS_GUI_STYLE_BORDER_WIDTH, .value = 2},
        {GS_GUI_STYLE_CONTENT_COLOR, .color = gs_color(67, 67, 67, 255)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(198, 198, 198, 255)}
    };

    gs_gui_animation_property_t button_animation[] = {
        // type, time, delay
        {GS_GUI_STYLE_HEIGHT, 100, 0},
        {GS_GUI_STYLE_BACKGROUND_COLOR, 200, 20},
        {GS_GUI_STYLE_MARGIN_TOP, 150, 0},
        {GS_GUI_STYLE_CONTENT_COLOR, 200, 0}
    };

    gs_gui_style_element_t button_hover_style[] = {
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(168, 168, 168, 255)},
        {GS_GUI_STYLE_HEIGHT, .value = 47}
    };

    gs_gui_style_element_t button_focus_style[] = {
        {GS_GUI_STYLE_FONT, .font = &app->fonts[GUI_FONT_BUTTONFOCUS]},
        {GS_GUI_STYLE_CONTENT_COLOR, .color = gs_color(255, 255, 255, 255)},
        {GS_GUI_STYLE_BACKGROUND_COLOR, .color = gs_color(49, 174, 31, 255)},
        {GS_GUI_STYLE_HEIGHT, .value = 50},
        {GS_GUI_STYLE_PADDING_BOTTOM, .value = 12}
    }; 

    gs_gui_style_element_t label_style[] = {
        {GS_GUI_STYLE_FONT, .font = &app->fonts[GUI_FONT_LABEL]},
        {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_END}
    }; 

    // Transitions get applied to 
    gs_gui_style_element_t text_style[] = {
        {GS_GUI_STYLE_FONT, .font = &app->fonts[GUI_FONT_LABEL]},
        {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
        {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_START}
    }; 

    // Generate new style sheet to use for menu
    app->menu_style_sheet = gs_gui_style_sheet_new(&app->gui, &(gs_gui_style_sheet_desc_t){
        .button = {
            .all = {
                .style = {button_style, sizeof(button_style)},
                .animation = {button_animation, sizeof(button_animation)}
            },
            .hover = {.style = {button_hover_style, sizeof(button_hover_style)}},
            .focus = {.style = {button_focus_style, sizeof(button_focus_style)}}
        },
        .panel = {
            .all = {.style = {panel_style, sizeof(panel_style)}}
        },
        .label = {
            .all = {.style = {label_style, sizeof(label_style)}}
        },
        .text = {
            .all = {.style = {text_style, sizeof(text_style)}}
        }
    }); 
} 

void app_update()
{
    // Cache data for frame
    app_t* app = gs_engine_user_data(app_t);
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
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 500, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMLEFT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_gui_draw_control_text(gui, str, next, &gui->style_sheet->styles[GS_GUI_ELEMENT_TEXT][0], 0x00); 
            }

            // Copyright
            {
                const char* str = "Copyright Gunslinger. Please, do distrubute.";
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 500, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_BOTTOMRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_gui_draw_control_text(gui, str, next, &gui->style_sheet->styles[GS_GUI_ELEMENT_LABEL][0], 0x00); 
            }

            // Frames
            {
                gs_snprintfc(TMP, 256, "frame: %.2f", gs_engine_subsystem(platform)->time.frame);
                gs_gui_layout_set_next(gui, gs_gui_layout_anchor(&cnt->body, 150, 50, 0, 0, GS_GUI_LAYOUT_ANCHOR_TOPRIGHT), 0); 
                gs_gui_rect_t next = gs_gui_layout_next(gui);
                gs_gui_draw_rect(gui, next, gs_color(0, 0, 0, 20));
                gs_gui_draw_control_text(gui, TMP, next, &gui->style_sheet->styles[GS_GUI_ELEMENT_LABEL][0], 0x00); 
            } 

            gs_gui_end_window(gui);
        } 

        if (gs_platform_key_pressed(GS_KEYCODE_F1)) debug_enabled = !debug_enabled; 

        // Set style sheet to default sheet
        gs_gui_set_style_sheet(gui, NULL); 
        if (gs_gui_begin_window_ex(gui, "Debug", gs_gui_rect(0, 0, 300, 200), &debug_enabled, 0x00))
        {
            gs_gui_end_window(gui);
        }

        style_window(gui, &app->menu_style_sheet, &debug_enabled);

        if (gs_gui_begin_window_ex(gui, "Tools", gs_gui_rect(0, 0, 300, 200), &debug_enabled, 0x00))
        {
            gs_gui_end_window(gui);
        }
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

static uint8_t uint8_slider(gs_gui_context_t *ctx, unsigned char *value, int low, int high) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = gs_gui_slider_ex(ctx, &tmp, low, high, 0, "%.0f", GS_GUI_OPT_ALIGNCENTER);
    *value = tmp;
    gs_gui_pop_id(ctx);
    return res;
}

static int32_t int32_slider(gs_gui_context_t *ctx, int32_t* value, int32_t low, int32_t high) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = gs_gui_slider_ex(ctx, &tmp, low, high, 0, "%.0f", GS_GUI_OPT_ALIGNCENTER);
    *value = tmp;
    gs_gui_pop_id(ctx);
    return res;
}

static int16_t int16_slider(gs_gui_context_t *ctx, int16_t* value, int32_t low, int32_t high) 
{
    static float tmp;
    gs_gui_push_id(ctx, &value, sizeof(value));
    tmp = *value;
    int res = gs_gui_slider_ex(ctx, &tmp, low, high, 0, "%.0f", GS_GUI_OPT_ALIGNCENTER);
    *value = tmp;
    gs_gui_pop_id(ctx);
    return res;
}

void style_window(gs_gui_context_t *ctx, gs_gui_style_sheet_t* style_sheet, bool* open) 
{
  static struct {const char* label; int32_t idx;} elements[] = {
    {"container",  GS_GUI_ELEMENT_CONTAINER},
    {"button",  GS_GUI_ELEMENT_BUTTON} ,
    {"panel",  GS_GUI_ELEMENT_PANEL},
    {"label",  GS_GUI_ELEMENT_LABEL},
    {"text",  GS_GUI_ELEMENT_TEXT},
    {"scroll",  GS_GUI_ELEMENT_SCROLL},
    {"base",  GS_GUI_ELEMENT_BASE},
    {NULL}
  }; 

  static char* states[] = {
      "default", 
      "hover", 
      "focus"
  };

  static struct {const char* label; int32_t idx;} colors[] = {
    {"background",  GS_GUI_COLOR_BACKGROUND},
    {"content",  GS_GUI_COLOR_CONTENT} ,
    {"border",  GS_GUI_COLOR_BORDER},
    {"shadow",  GS_GUI_COLOR_SHADOW},
    {NULL}
  }; 

  if (gs_gui_begin_window_ex(ctx, "Style Editor", gs_gui_rect(350, 250, 300, 240), open, 0x00)) 
  { 
    for (uint32_t i = 0; elements[i].label; ++i)
    {
        int32_t idx = elements[i].idx; 

        if (gs_gui_begin_treenode_ex(ctx, elements[i].label, 0x00))
        {
            for (uint32_t j = 0; j < GS_GUI_ELEMENT_STATE_COUNT; ++j)
            {
                gs_gui_push_id(ctx, &j, sizeof(j));
                gs_gui_style_t* s = &style_sheet->styles[idx][j];
                if (gs_gui_begin_treenode_ex(ctx, states[j], 0x00))
                {
                    gs_gui_style_t* save = gs_gui_push_style(ctx, &ctx->style_sheet->styles[GS_GUI_ELEMENT_PANEL][0x00]);
                    gs_gui_layout_row(ctx, 1, (int[]){-1}, 300);
                    gs_gui_begin_panel(ctx, states[j]);
                    {
                        gs_gui_layout_t* l = gs_gui_get_layout(ctx);
                        gs_gui_rect_t* r = &l->body; 

                        const float ls = 80;

                        // size
                        float w = (l->body.w - ls) * 0.35f;
                        gs_gui_layout_row(ctx, 3, (int[]) {ls, w, w}, 0); 
                        gs_gui_label(ctx, "size:");
                        gs_gui_slider(ctx, &s->size[0], 0.f, 100.f);
                        gs_gui_slider(ctx, &s->size[1], 0.f, 100.f); 

                        // spacing
                        w = (l->body.w - ls) * 0.7f;
                        gs_gui_layout_row(ctx, 2, (int[]) {ls, w}, 0); 
                        gs_gui_label(ctx, "spacing:");
                        gs_gui_slider(ctx, &s->spacing, 0.f, 100.f);

                        gs_gui_label(ctx, "indent:");
                        int16_slider(ctx, &s->indent, 0.f, 100.f);

                        gs_gui_label(ctx, "scrollbar_size:");
                        int16_slider(ctx, &s->scrollbar_size, 0.f, 100.f);

                        gs_gui_label(ctx, "title_height:");
                        int16_slider(ctx, &s->title_height, 0.f, 100.f);

                        gs_gui_label(ctx, "thumb_size:");
                        int16_slider(ctx, &s->thumb_size, 0.f, 100.f); 

                        gs_gui_label(ctx, "border_width:");
                        int16_slider(ctx, &s->border_width, 0.f, 100.f); 

                        gs_gui_label(ctx, "border_radius:");
                        int16_slider(ctx, &s->border_radius, 0.f, 100.f); 

                        // padding/margin
                        w = (l->body.w - ls) * 0.2f;
                        gs_gui_layout_row(ctx, 5, (int[]) {ls, w, w, w, w}, 0); 
                        gs_gui_label(ctx, "padding:");
                        int32_slider(ctx, &s->padding[0], 0.f, 100.f); 
                        int32_slider(ctx, &s->padding[1], 0.f, 100.f); 
                        int32_slider(ctx, &s->padding[2], 0.f, 100.f); 
                        int32_slider(ctx, &s->padding[3], 0.f, 100.f); 

                        gs_gui_label(ctx, "margin:");
                        int16_slider(ctx, &s->margin[0], 0.f, 100.f); 
                        int16_slider(ctx, &s->margin[1], 0.f, 100.f); 
                        int16_slider(ctx, &s->margin[2], 0.f, 100.f); 
                        int16_slider(ctx, &s->margin[3], 0.f, 100.f); 

                        // Colors
                        int sw = (int32_t)(l->body.w * 0.14);
                        gs_gui_layout_row(ctx, 6, (int[]) {80, sw, sw, sw, sw, -1}, 0);
                        for (uint32_t c = 0; c < GS_GUI_COLOR_MAX; ++c)
                        {
                            gs_gui_label(ctx, colors[c].label);
                            uint8_slider(ctx, &s->colors[c].r, 0, 255);
                            uint8_slider(ctx, &s->colors[c].g, 0, 255);
                            uint8_slider(ctx, &s->colors[c].b, 0, 255);
                            uint8_slider(ctx, &s->colors[c].a, 0, 255);
                            gs_gui_draw_rect(ctx, gs_gui_layout_next(ctx), s->colors[c]);
                        }
                    }
                    gs_gui_end_panel(ctx); 
                    gs_gui_pop_style(ctx, save);

                    gs_gui_end_treenode(ctx);
                }
                gs_gui_pop_id(ctx);
            }
            gs_gui_end_treenode(ctx);
        }
    } 
    gs_gui_end_window(ctx);
  }
}

void app_shutdown()
{
    // free gui
    app_t* app = gs_engine_user_data(app_t);
    gs_gui_free(&app->gui);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .user_data = gs_malloc_init(app_t),
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown,
        .window_width = 900,
        .window_height = 580,
        .frame_rate = 60
    };
}


