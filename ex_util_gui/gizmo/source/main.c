/*================================================================
    * Copyright: 2020 John Jackson
    * gs_gui gizmo example

    The purpose of this example is to demonstrate how to use the gs_gui_gizmo 
    widget.

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
    gs_camera_t camera;
    gs_vqs xform;
} app_t; 

void dockspace(gs_gui_context_t* ctx);
void gui_cb(gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd);

void app_init()
{
    app_t* app = gs_user_data(app_t);
    app->cb = gs_command_buffer_new(); 
    gs_gui_init(&app->gui, gs_platform_main_window());
    app->camera = gs_camera_perspective();
	app->xform = (gs_vqs) {
		.translation = gs_v3s(0.f),
		.rotation = gs_quat_mul_list(2, 
			gs_quat_angle_axis(gs_deg2rad(45.f), GS_YAXIS),
			gs_quat_angle_axis(gs_deg2rad(45.f), GS_XAXIS)
		),
		.scale = gs_v3s(0.5f)
	};

	// Set up initial dock
	// gs_gui_dock_ex(&app->gui, "Canvas", "Dockspace", GS_GUI_SPLIT_BOTTOM, 1.f);
} 

void app_update()
{
    app_t* app = gs_user_data(app_t);
    gs_command_buffer_t* cb = &app->cb;
    gs_gui_context_t* gui = &app->gui;
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) {
        gs_quit();
    }

    // Begin new frame for gui
    gs_gui_begin(gui, fbs);

    dockspace(gui); 

    const gs_vec2 ws = gs_v2(800.f, 600.f);
    gs_gui_window_begin(gui, "Canvas", gs_gui_rect((fbs.x - ws.x) * 0.5f, (fbs.y - ws.y) * 0.5f, ws.x, ws.y));
    {
        // Cache the current container
        gs_gui_container_t* cnt = gs_gui_get_current_container(gui);

        static int32_t gizmo_op = GS_GUI_GIZMO_ROTATE;
        static int32_t gizmo_mode = GS_GUI_TRANSFORM_LOCAL;
		static float snap[3] = {0}; 

        // Set the layout to take up the entire window space
        gs_gui_layout_row(gui, 1, (int[]){-1}, -1);
        gs_gui_gizmo(gui, &app->camera, &app->xform, snap[gizmo_op], gizmo_op, gizmo_mode, GS_GUI_OPT_LEFTCLICKONLY);

        // Do overlay for gizmo options, anchored to the top left of the window
        gs_gui_rect_t anchor = gs_gui_layout_anchor(&cnt->body, cnt->body.w, 200, 1, 1, GS_GUI_LAYOUT_ANCHOR_TOPLEFT);
        gs_gui_layout_set_next(gui, anchor, 0);

        // We want a row with two drop down combo boxes, one for the op and one for the mode
        struct {const char* str; int32_t option;} ops[] = {
            {"Translate", GS_GUI_GIZMO_TRANSLATE},
            {"Rotate", GS_GUI_GIZMO_ROTATE},
            {"Scale", GS_GUI_GIZMO_SCALE},
            {NULL}
        };

        struct {const char* str; int32_t option;} modes[] = {
            {"World", GS_GUI_TRANSFORM_WORLD},
            {"Local", GS_GUI_TRANSFORM_LOCAL},
            {NULL}
        };

		struct {const char* str; float* option;} snaps[] = {
			{"Translation", &snap[0]},
			{"Rotation", &snap[1]},
			{"Scale", &snap[2]},
			{NULL}
		};

		// Panel for gizmo options
		gs_gui_panel_begin_ex(gui, "#options", NULL, GS_GUI_OPT_NOFRAME | GS_GUI_OPT_NOINTERACT);
		{
			gs_gui_layout_row(gui, 3, (int[]){120, 120, 180}, 20);

            char TMP[256] = {0};

            gs_snprintf(TMP, sizeof(TMP), "Op: %s", ops[gizmo_op].str);
			if (gs_gui_combo_begin(gui, "#op", TMP, 3)) {
				gs_gui_layout_row(gui, 1, (int[]) {-1}, 0); 
				for (uint32_t i = 0; ops[i].str != NULL; ++i) {
					if (gs_gui_button(gui, ops[i].str)) gizmo_op = i;
				}
				gs_gui_combo_end(gui);
			} 

            gs_snprintf(TMP, sizeof(TMP), "Mode: %s", modes[gizmo_mode].str);
			if (gs_gui_combo_begin(gui, "#mode", TMP, 2)) {
				gs_gui_layout_row(gui, 1, (int[]) {-1}, 0);
				for (uint32_t i = 0; modes[i].str != NULL; ++i) {
					if (gs_gui_button(gui, modes[i].str)) gizmo_mode = i;
				}
				gs_gui_combo_end(gui);
			} 

            if (gs_gui_combo_begin(gui, "#snaps", "Snap", 3))
            {
                gs_gui_push_id(gui, "#snap", strlen("#snap"));
                gs_gui_layout_row(gui, 2, (int[]){100, -5}, 0);
                for (uint32_t i = 0; snaps[i].str != NULL; ++i) {
                    gs_gui_label(gui, snaps[i].str);
                    gs_gui_number(gui, snaps[i].option, 0.01f);
                }
                gs_gui_pop_id(gui);
                gs_gui_combo_end(gui);
            }

		}
		gs_gui_panel_end(gui);

		// Custom callback for transformed object drawn into window
		gs_gui_draw_custom(gui, cnt->body, gui_cb, NULL, 0);
    } 
    gs_gui_window_end(gui);

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

void dockspace(gs_gui_context_t* ctx) 
{
    int32_t opt = 
		GS_GUI_OPT_NOCLIP | 
		GS_GUI_OPT_NOFRAME | 
		GS_GUI_OPT_FORCESETRECT | 
		GS_GUI_OPT_NOTITLE | 
		GS_GUI_OPT_DOCKSPACE | 
		GS_GUI_OPT_FULLSCREEN | 
		GS_GUI_OPT_NOMOVE | 
		GS_GUI_OPT_NOBRINGTOFRONT | 
		GS_GUI_OPT_NOFOCUS | 
		GS_GUI_OPT_NORESIZE;

    gs_gui_window_begin_ex(ctx, "Dockspace", gs_gui_rect(350, 40, 600, 500), NULL, NULL, opt);
    {
        // Empty dockspace...
    }
    gs_gui_window_end(ctx);
} 

// Custom callback for immediate drawing directly into the gui window
void gui_cb( gs_gui_context_t* ctx, struct gs_gui_customcommand_t* cmd )
{
    app_t* app = gs_user_data(app_t);	
    gs_immediate_draw_t* gsi = &ctx->gsi;   // Immediate draw list in gui context
    gs_vec2 fbs = ctx->framebuffer_size;    // Framebuffer size bound for gui context

    // Set up an immedaite camera using our passed in cmd viewport (this is the clipped viewport of the gui window being drawn)
    gsi_defaults(gsi);
    gsi_camera3D(gsi, (uint32_t)cmd->viewport.w, (uint32_t)cmd->viewport.h); 
    gs_graphics_set_viewport(&gsi->commands, cmd->viewport.x, fbs.y - cmd->viewport.h - cmd->viewport.y, cmd->viewport.w, cmd->viewport.h); 
    gsi_push_matrix(gsi, GSI_MATRIX_MODELVIEW);
    {
        gsi_mul_matrix(gsi, gs_vqs_to_mat4(&app->xform));
        gsi_box(gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
    }
    gsi_pop_matrix(gsi);
}












