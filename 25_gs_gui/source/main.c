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

//For console window
static   int logbuf_updated = 0;
static  char logbuf[64000];

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
} 

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) 
    {
        gs_engine_quit();
    }

    // Begin new frame for gui
    gs_gui_begin(&gsgui);

    dockspace(&gsgui);
    log_window(&gsgui);
    test_window(&gsgui);

    // End gui frame
    gs_gui_end(&gsgui);

    // Do rendering
    gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = {bg[0]/255, bg[1]/255,bg[2]/255, 1.f}}};
    gs_graphics_begin_render_pass(&gcb, (gs_handle(gs_graphics_render_pass_t)){0});
    {
        gs_graphics_clear(&gcb, &clear);
        gs_graphics_set_viewport(&gcb,0,0,(int)fbs.x,(int)fbs.y);
        gs_gui_render(&gsgui, &gcb);
    }
    gs_graphics_end_render_pass(&gcb);
    
    //Submits to cb
    gs_graphics_submit_command_buffer(&gcb);
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
        .window_width = 1024,
        .window_height = 760
    };
}

void dockspace(gs_gui_context_t* ctx) 
{
    int32_t opt = GS_GUI_OPT_NOCLIP | GS_GUI_OPT_NOFRAME | GS_GUI_OPT_FORCESETRECT | GS_GUI_OPT_NOTITLE | GS_GUI_OPT_DOCKSPACE | GS_GUI_OPT_FULLSCREEN | GS_GUI_OPT_NOMOVE | GS_GUI_OPT_NOBRINGTOFRONT | GS_GUI_OPT_NOFOCUS | GS_GUI_OPT_NORESIZE;
    gs_gui_begin_window_ex(ctx, "Dockspace", gs_gui_rect(350, 40, 600, 500), NULL, opt);
    {
        // Empty dockspace...
    }
    gs_gui_end_window(ctx);
}

void write_log(const char* text) 
{
    /* FIXME: THIS IS UNSAFE! */
    if (logbuf[0]) 
    {
        strcat(logbuf, "\n");
    }
    strcat(logbuf, text);
    logbuf_updated = 1;
}

void log_window(gs_gui_context_t* ctx) 
{
    if (gs_gui_begin_window(ctx, "Log Window", gs_gui_rect(350, 40, 300, 200))) 
    {
        /* output text panel */
        gs_gui_layout_row(ctx, 1, (int[]) { -1 }, -50);
        gs_gui_begin_panel(ctx, "Log Output");
        gs_gui_container_t* panel = gs_gui_get_current_container(ctx);
        gs_gui_layout_row(ctx, 1, (int[]) { -1 }, -1);
        gs_gui_text(ctx, logbuf, 0);
        gs_gui_end_panel(ctx);

        if (logbuf_updated) 
        {
            panel->scroll.y = panel->content_size.y;
            logbuf_updated = 0;
        }

        /* input textbox + submit button */
        static char buf[128];
        int submitted = 0;
        gs_gui_layout_row(ctx, 2, (int[]) { -70, -1 }, 0);
        if (gs_gui_textbox(ctx, buf, sizeof(buf)) & GS_GUI_RES_SUBMIT) 
        {
            gs_gui_set_focus(ctx, ctx->last_id);
            submitted = 1;
        }
        if (gs_gui_button(ctx, "Submit")) { submitted = 1; }
        if (submitted) 
        {
            write_log(buf);
            buf[0] = '\0';
        }

        gs_gui_end_window(ctx);
    }
}

void test_window(gs_gui_context_t *ctx) 
{
    /* do window */
    if (gs_gui_begin_window(ctx, "Demo", gs_gui_rect(40, 40, 300, 450)))
    {
        if (gs_gui_header(ctx, "Window Info")) 
        {
            gs_gui_container_t* win = gs_gui_get_current_container(ctx);
            char buf[64];
            gs_gui_layout_row(ctx, 2, (int[]) { 65, -1 }, 0);
            gs_gui_label(ctx,"Position:");
            gs_snprintf(buf, 64, "%.2f, %.2f", win->rect.x, win->rect.y); gs_gui_label(ctx, buf);
            gs_gui_label(ctx, "Size:");
            gs_snprintf(buf, 64, "%.2f, %.2f", win->rect.w, win->rect.h); gs_gui_label(ctx, buf);
        }

        if (gs_gui_header_ex(ctx, "Test Buttons", 0x00)) 
        {
            gs_gui_layout_row(ctx, 3, (int[]) { 120, 100, 100 }, 0);
            gs_gui_label(ctx, "Test buttons 1:");
            if (gs_gui_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
            if (gs_gui_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
            gs_gui_label(ctx, "Test buttons 2:");
            if (gs_gui_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
            if (gs_gui_button(ctx, "Popup")) 
			{ 
				gs_gui_open_popup(ctx, "Test Popup"); 
			}
			if (gs_gui_begin_popup( ctx, "Test Popup", gs_gui_rect(ctx->mouse_pos.x, ctx->mouse_pos.y, 100, 100))) 
			{
                gs_gui_button(ctx, "Hello");
                gs_gui_button(ctx, "World");
                gs_gui_end_popup(ctx);
            }
        }

        if (gs_gui_header_ex(ctx, "Tree and Text", 0x00)) 
        {
            gs_gui_layout_row(ctx, 2, (int[]) { 250, -1 }, 0);
            gs_gui_layout_begin_column(ctx);
            if (gs_gui_begin_treenode(ctx, "Test 1")) {
                if (gs_gui_begin_treenode(ctx, "Test 1a")) {
                    gs_gui_label(ctx, "Hello");
                    gs_gui_label(ctx, "world");
                    gs_gui_end_treenode(ctx);
                }
                if (gs_gui_begin_treenode(ctx, "Test 1b")) {
                    if (gs_gui_button(ctx, "Button 1")) { write_log("Pressed button 1"); }
                    if (gs_gui_button(ctx, "Button 2")) { write_log("Pressed button 2"); }
                    gs_gui_end_treenode(ctx);
                }
                gs_gui_end_treenode(ctx);
            }

            if (gs_gui_begin_treenode(ctx, "Test 2")) 
            {
                gs_gui_layout_row(ctx, 3, (int[]) { 65, 65, 65 }, 0);
                if (gs_gui_button(ctx, "Button 3")) { write_log("Pressed button 3"); }
                if (gs_gui_button(ctx, "Button 4")) { write_log("Pressed button 4"); }
                if (gs_gui_button(ctx, "Button 5")) { write_log("Pressed button 5"); }
                if (gs_gui_button(ctx, "Button 6")) { write_log("Pressed button 6"); }
                gs_gui_end_treenode(ctx);
            }

            if (gs_gui_begin_treenode(ctx, "Test 3")) 
            {
                static int checks[3] = { 1, 0, 1 };
                gs_gui_checkbox(ctx, "Checkbox 1", &checks[0]);
                gs_gui_checkbox(ctx, "Checkbox 2", &checks[1]);
                gs_gui_checkbox(ctx, "Checkbox 3", &checks[2]);
                gs_gui_end_treenode(ctx);
            }
            gs_gui_layout_end_column(ctx);

            gs_gui_layout_begin_column(ctx);
            gs_gui_layout_row(ctx, 1, (int[]) { -1 }, 0);
            gs_gui_text(ctx, "Lorem ipsum dolor sit amet, consectetur adipiscing "
                         "elit. Maecenas lacinia, sem eu lacinia molestie, mi risus faucibus "
                         "ipsum, eu varius magna felis a nulla.", 1);
            gs_gui_layout_end_column(ctx);
        }

        if (gs_gui_header_ex(ctx, "Background Color", 0x00)) 
        {
            gs_gui_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);
            gs_gui_layout_begin_column(ctx);
            gs_gui_layout_row(ctx, 2, (int[]) { 46, -1 }, 0);
            gs_gui_label(ctx, "Red:");   gs_gui_slider(ctx, &bg[0], 0, 255);
            gs_gui_label(ctx, "Green:"); gs_gui_slider(ctx, &bg[1], 0, 255);
            gs_gui_label(ctx, "Blue:");  gs_gui_slider(ctx, &bg[2], 0, 255);
            gs_gui_layout_end_column(ctx);
            gs_gui_rect_t r = gs_gui_layout_next(ctx);
            gs_gui_draw_rect(ctx, r, gs_color(bg[0], bg[1], bg[2], 255));
            char buf[32];
            gs_snprintf(buf, 32, "#%02X%02X%02X", (int) bg[0], (int) bg[1], (int) bg[2]);
            gs_gui_draw_control_text(ctx, buf, r, &ctx->style_sheet->styles[GS_GUI_ELEMENT_LABEL][0x00], GS_GUI_OPT_ALIGNCENTER);
        }

        if (gs_gui_header_ex(ctx, "Test", 0x00)) 
        {
            gs_snprintfc(buf, 32, "buffer");
            gs_gui_layout_row(ctx, 3, (int[]) { 30, -150, -1 }, 0);
            gs_gui_button(ctx, "X");
            gs_gui_textbox(ctx, buf, sizeof(buf));
            gs_gui_button(ctx, "Submit");
        }

        gs_gui_end_window(ctx);
    }
}

