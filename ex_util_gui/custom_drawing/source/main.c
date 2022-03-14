/*================================================================
    * Copyright: 2020 John Jackson
    * gs_gui custom drawing example

    The purpose of this example is to demonstrate how to use the gs_gui_draw_custom 
    function for custom drawing into a gui window.

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
} app_t; 

void gui_cb(gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd);

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new(); 
    gs_gui_init(&app->gui, gs_platform_main_window()); 
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

    // Initial sizes for windows
    const gs_vec2 ws = gs_v2(300.f, 200.f);

    gs_gui_window_begin(gui, "Single", gs_gui_rect((fbs.x - ws.x) * 0.2f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
    {
        // Cache the current container
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui); 

		// Custom callback for transformed object drawn into this current window
        // Here we're passing in the container's body as the viewport, but this can be whatever you'd like, 
        // as we'll see in the "split" window example.
        // We're passing in custom data (color) as well so we can use that in our callback. This can be NULL if 
        // you don't require anything.  
        gs_color_t color = gs_color_alpha(GS_COLOR_RED, (uint8_t)gs_clamp((sin(t * 0.001f) * 0.5f + 0.5f)* 255, 0, 255));
		gs_gui_draw_custom(gui, cnt->body, gui_cb, &color, sizeof(color)); 
    } 
    gs_gui_window_end(gui);

    gs_gui_window_begin(gui, "Split", gs_gui_rect((fbs.x - ws.x) * 0.8f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
    {
        // Cache the current container
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui); 

        // We'll do the same callback here, but this time we'll split the window into two separate viewports using 
        // the auto layout system to draw twice.
        float w = cnt->body.w * 0.5f - 5;
        gs_gui_layout_row(gui, 2, (int[]){w, w}, -1);   // Split the window into half, take up the full height by setting to -1
		gs_gui_draw_custom(gui, gs_gui_layout_next(gui), gui_cb, &(gs_color_t){0, 255, 0, 255}, sizeof(gs_color_t)); 
		gs_gui_draw_custom(gui, gs_gui_layout_next(gui), gui_cb, &(gs_color_t){255, 200, 0, 255}, sizeof(gs_color_t)); 
    } 
    gs_gui_window_end(gui);

    // End gui frame
    gs_gui_end(gui);

    // Do rendering
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {0.05f, 0.05f, 0.05f, 1.f}}};
    gs_graphics_renderpass_begin(cb, (gs_handle(gs_graphics_renderpass_t)){0});
    {
        gs_graphics_clear(cb, &clear);
        gs_graphics_set_viewport(cb,0,0,(int)fbs.x,(int)fbs.y);
        gs_gui_render(gui, cb);
    }
    gs_graphics_renderpass_end(cb);
    
    //Submits to cb
    gs_graphics_command_buffer_submit(cb);
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

// Custom callback for immediate drawing directly into the gui window
void gui_cb( gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd )
{
    gs_immediate_draw_t* gsi = &ctx->gsi;          // Immediate draw list in gui context
    gs_vec2 fbs = ctx->framebuffer_size;           // Framebuffer size bound for gui context
    gs_color_t* color = (gs_color_t*)cmd->data;    // Grab custom data
    const float t = gs_platform_elapsed_time(); 

    // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
    gsi_camera3D(gsi, (uint32_t)cmd->viewport.w, (uint32_t)cmd->viewport.h); 
    gsi_blend_enabled(gsi, true);
    gs_graphics_set_viewport(&gsi->commands, cmd->viewport.x, fbs.y - cmd->viewport.h - cmd->viewport.y, cmd->viewport.w, cmd->viewport.h); 
    gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_rotatev(gsi, t * 0.001f, GS_YAXIS);
        gsi_scalef(gsi, 0.5f, 0.5f, 0.5f);
        gsi_box(gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, color->r, color->g, color->b, color->a, GS_GRAPHICS_PRIMITIVE_LINES);
    }
    gsi_pop_matrix(gsi);
}












