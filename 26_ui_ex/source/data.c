/*================================================================
    * Copyright: 2020 John Jackson
    * simple custom ui example data sheet
================================================================*/ 

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

app_t g_app = {0}; 

// Set up styles for elements 
gs_gui_style_element_t panel_style[] = {
    {GS_GUI_STYLE_PADDING_TOP, .value = 20},
    {GS_GUI_STYLE_BORDER_COLOR, .color = {0, 0, 0, 0}},
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {0, 0, 0, 0}}
};

gs_gui_style_element_t button_style[] = {
    // type, value
    {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
    {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_CENTER},
    {GS_GUI_STYLE_WIDTH, .value = 200},
    {GS_GUI_STYLE_HEIGHT, .value = 45},
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_BUTTON]},
    {GS_GUI_STYLE_MARGIN_LEFT, .value = 0},
    {GS_GUI_STYLE_MARGIN_TOP, .value = 10}, 
    {GS_GUI_STYLE_MARGIN_BOTTOM, .value = 0},
    {GS_GUI_STYLE_MARGIN_RIGHT, .value = 20},
    {GS_GUI_STYLE_SHADOW_X, .value = 1},
    {GS_GUI_STYLE_SHADOW_Y, .value = 1}, 
    {GS_GUI_STYLE_SHADOW_COLOR, .color = {146, 146, 146, 200}},
    {GS_GUI_STYLE_BORDER_COLOR, .color = {0, 0, 0, 255}},
    {GS_GUI_STYLE_BORDER_WIDTH, .value = 2},
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {67, 67, 67, 255}},
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {198, 198, 198, 255}}
};

gs_gui_animation_property_t button_animation[] = {
    // type, time, delay
    {GS_GUI_STYLE_HEIGHT, 100, 0},
    {GS_GUI_STYLE_BACKGROUND_COLOR, 200, 20},
    {GS_GUI_STYLE_MARGIN_TOP, 150, 0},
    {GS_GUI_STYLE_CONTENT_COLOR, 200, 0}
};

gs_gui_style_element_t button_hover_style[] = {
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {168, 168, 168, 255}},
    {GS_GUI_STYLE_HEIGHT, .value = 47}
};

gs_gui_style_element_t button_focus_style[] = {
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_BUTTONFOCUS]},
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {255, 255, 255, 255}},
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {49, 174, 31, 255}},
    {GS_GUI_STYLE_HEIGHT, .value = 50},
    {GS_GUI_STYLE_PADDING_BOTTOM, .value = 12}
}; 

gs_gui_style_element_t label_style[] = {
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_LABEL]},
    {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
    {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_START}
}; 

gs_gui_animation_property_t label_animation[] = {
    // type, time, delay
    {GS_GUI_STYLE_CONTENT_COLOR, 100, 0},
    {GS_GUI_STYLE_BACKGROUND_COLOR, 100, 0},
};

gs_gui_style_element_t label_hover_style[] = {
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {255, 0, 0, 255}}
}; 

gs_gui_style_element_t label_focus_style[] = {
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {0, 255, 0, 255}}
}; 

// Transitions get applied to 
gs_gui_style_element_t text_style[] = {
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_LABEL]},
    {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
    {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_START}
}; 

// Style sheet desc
gs_gui_style_sheet_desc_t menu_style_sheet_desc = {
    .button = {
        .all = {
            .style = {button_style, sizeof(button_style)},
            .animation = {button_animation, sizeof(button_animation)}
        },
        .hover = {.style = {button_hover_style, sizeof(button_hover_style)}},
        .focus = {.style = {button_focus_style, sizeof(button_focus_style)}},
    },
    .panel = {
        .all = {.style = {panel_style, sizeof(panel_style)}}
    },
    .label = {
        .all = {
            .style = {label_style, sizeof(label_style)},
            .animation = {label_animation, sizeof(label_animation)}
        },
        .hover = {.style = {label_hover_style, sizeof(label_hover_style)}},
        .focus = {.style = {label_focus_style, sizeof(label_focus_style)}}
    },
    .text = {
        .all = {.style = {text_style, sizeof(text_style)}}
    }
}; 

// Inline style desc for custom button
gs_gui_style_element_t inline_btn_style[] = { 
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {255, 0, 0, 255}},
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {0, 255, 0, 255}}
};
gs_gui_style_element_t inline_btn_hov_style[] = { 
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {255, 100, 0, 255}},
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {255, 255, 255, 255}},
    {GS_GUI_STYLE_HEIGHT, 60}
}; 
gs_gui_style_element_t inline_btn_foc_style[] = { 
    {GS_GUI_STYLE_BACKGROUND_COLOR, .color = {0, 0, 255, 255}},
    {GS_GUI_STYLE_CONTENT_COLOR, .color = {0, 255, 0, 255}}
}; 
gs_gui_animation_property_t inline_btn_hov_anim[] = { 
    // type, time, delay
    {GS_GUI_STYLE_HEIGHT, 1000, 0}
};
gs_gui_animation_property_t inline_btn_anim[] = { 
    // type, time, delay
    {GS_GUI_STYLE_BACKGROUND_COLOR, 0, 0},
    {GS_GUI_STYLE_MARGIN_TOP, 0, 0},
    {GS_GUI_STYLE_CONTENT_COLOR, 0, 0}
};

// inline style desc
gs_gui_inline_style_desc_t btn_inline_style = {
    .all = {
        .style = {.data = inline_btn_style, .size = sizeof(inline_btn_style)},
        .animation = {.data = inline_btn_anim, .size = sizeof(inline_btn_anim)} 
    },
    .hover = {
        .style = {.data = inline_btn_hov_style, .size = sizeof(inline_btn_hov_style)},
        .animation = {.data = inline_btn_hov_anim, .size = sizeof(inline_btn_hov_anim)} 
    },
    .focus = {
        .style = {.data = inline_btn_foc_style, .size = sizeof(inline_btn_foc_style)}
    }
};


