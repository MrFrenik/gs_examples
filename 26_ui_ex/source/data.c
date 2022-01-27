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
    {GS_GUI_STYLE_COLOR_BORDER, .color = {0, 0, 0, 0}},
    {GS_GUI_STYLE_COLOR_BACKGROUND, .color = {0, 0, 0, 0}}
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
    {GS_GUI_STYLE_COLOR_SHADOW, .color = {146, 146, 146, 200}},
    {GS_GUI_STYLE_COLOR_BORDER, .color = {0, 0, 0, 255}},
    {GS_GUI_STYLE_BORDER_WIDTH, .value = 2},
    {GS_GUI_STYLE_COLOR_CONTENT, .color = {67, 67, 67, 255}},
    {GS_GUI_STYLE_COLOR_BACKGROUND, .color = {198, 198, 198, 255}}
};

gs_gui_animation_property_t button_animation[] = {
    // type, time, delay
    {GS_GUI_STYLE_HEIGHT, 100, 0},
    {GS_GUI_STYLE_COLOR_BACKGROUND, 200, 20},
    {GS_GUI_STYLE_MARGIN_TOP, 150, 0},
    {GS_GUI_STYLE_COLOR_CONTENT, 200, 0}
};

gs_gui_style_element_t button_hover_style[] = {
    {GS_GUI_STYLE_COLOR_BACKGROUND, .color = {168, 168, 168, 255}},
    {GS_GUI_STYLE_HEIGHT, .value = 47}
};

gs_gui_style_element_t button_focus_style[] = {
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_BUTTONFOCUS]},
    {GS_GUI_STYLE_COLOR_CONTENT, .color = {255, 255, 255, 255}},
    {GS_GUI_STYLE_COLOR_BACKGROUND, .color = {49, 174, 31, 255}},
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
    {GS_GUI_STYLE_COLOR_CONTENT, 100, 0},
    {GS_GUI_STYLE_COLOR_BACKGROUND, 100, 0},
};

gs_gui_style_element_t label_hover_style[] = {
    {GS_GUI_STYLE_COLOR_CONTENT, .color = {255, 0, 0, 255}}
}; 

gs_gui_style_element_t label_focus_style[] = {
    {GS_GUI_STYLE_COLOR_CONTENT, .color = {0, 255, 0, 255}}
}; 

// Transitions get applied to 
gs_gui_style_element_t text_style[] = {
    {GS_GUI_STYLE_FONT, .font = &g_app.fonts[GUI_FONT_LABEL]},
    {GS_GUI_STYLE_ALIGN_CONTENT, .value = GS_GUI_ALIGN_CENTER},
    {GS_GUI_STYLE_JUSTIFY_CONTENT, .value = GS_GUI_JUSTIFY_START}
}; 

gs_gui_style_element_t scroll_style[] = {
    {GS_GUI_STYLE_WIDTH, 8}
}; 

gs_gui_style_element_t scroll_hov_style[] = {
    {GS_GUI_STYLE_WIDTH, 12}
}; 

gs_gui_animation_property_t scroll_animation[] = {
    // type, time, delay
    {GS_GUI_STYLE_WIDTH, 150, 0},
    {GS_GUI_STYLE_COLOR_BACKGROUND, 200, 20}
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
    },
    .scroll = {
        .all = {.animation = {scroll_animation, sizeof(scroll_animation)}},
        .def = {.style = {scroll_style, sizeof(scroll_style)}},
        .hover = {.style = {scroll_hov_style, sizeof(scroll_hov_style)}},
        .focus = {.style = {scroll_hov_style, sizeof(scroll_hov_style)}}
    }
}; 

#define BTN_HEIGHT      50
#define BTN_MARGIN_TOP  5
#define BTN_MARGIN_ADD  5

// Inline style desc for custom button
gs_gui_style_element_t inline_btn_style[] = { 
    {GS_GUI_STYLE_COLOR_BACKGROUND, .color = {255, 50, 0, 255}},
    {GS_GUI_STYLE_COLOR_CONTENT, .color = {255, 255, 255, 255}},
    {GS_GUI_STYLE_HEIGHT, .value = BTN_HEIGHT},
    {GS_GUI_STYLE_MARGIN_TOP, .value = BTN_MARGIN_TOP},
    {GS_GUI_STYLE_FONT, .value = &g_app.fonts[GUI_FONT_BUTTON]}
};
gs_gui_style_element_t inline_btn_foc_style[] = { 
    {GS_GUI_STYLE_HEIGHT, .value = BTN_HEIGHT - BTN_MARGIN_TOP},
    {GS_GUI_STYLE_MARGIN_TOP, .value = BTN_MARGIN_TOP + BTN_MARGIN_ADD}
}; 
gs_gui_animation_property_t inline_btn_anim[] = { 
    // type, time, delay
    {GS_GUI_STYLE_COLOR_BACKGROUND, 0, 0},
    {GS_GUI_STYLE_COLOR_CONTENT, 0, 0}
};
gs_gui_animation_property_t inline_btn_foc_anim[] = { 
    // type, time, delay
    {GS_GUI_STYLE_HEIGHT, 20, 0},
    {GS_GUI_STYLE_MARGIN_TOP, 20, 0} 
};

// inline style desc
gs_gui_inline_style_desc_t btn_inline_style = {
    .all = {
        .style = {.data = inline_btn_style, .size = sizeof(inline_btn_style)},
        .animation = {.data = inline_btn_anim, .size = sizeof(inline_btn_anim)} 
    },
    .focus = {
        .style = {.data = inline_btn_foc_style, .size = sizeof(inline_btn_foc_style)},
        .animation = {.data = inline_btn_foc_anim, .size = sizeof(inline_btn_foc_anim)} 
    }
};


