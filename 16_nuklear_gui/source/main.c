/*================================================================
    * Copyright: 2020 John Jackson
    * nuklear_gui

    The purpose of this example is to 

    Included: 
        *

    Press `esc` to exit the application.
================================================================*/

// Nuklear will define these, so we will not implement them ourselves
#include "impl.c"

gs_command_buffer_t gcb = {0};

/* Platform */
gs_nk_ctx_t         gs_nk = {0};
struct nk_colorf    bg = {0};
struct node_editor  editor = {0};

void init()
{
    gcb = gs_command_buffer_new();

    // Initialize gs_nk context
    gs_nk_init(&gs_nk, gs_platform_main_window(), 0x00);

    // Default font (can add different ones in here)
    gs_nk_font_stash_begin(&gs_nk, NULL);
    gs_nk_font_stash_end(&gs_nk);
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    struct nk_context* ctx = &gs_nk.nk_ctx;

    gs_nk_new_frame(&gs_nk);

    if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
    {
        enum {EASY, HARD};
        static int op = EASY;
        static int property = 20;
        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "button"))
            fprintf(stdout, "button pressed\n");

        nk_layout_row_dynamic(ctx, 30, 2);
        if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
        if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

        nk_layout_row_dynamic(ctx, 25, 1);
        nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "background:", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 25, 1);
        if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
            nk_layout_row_dynamic(ctx, 120, 1);
            bg = nk_color_picker(ctx, bg, NK_RGBA);
            nk_layout_row_dynamic(ctx, 25, 1);
            bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
            bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
            bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
            bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
            nk_combo_end(ctx);
        }
    }
    nk_end(ctx);

    calculator(ctx);

    node_editor(ctx, &editor);

    // Render gs_nk commands into graphics command buffer
    gs_nk_render(&gs_nk, &gcb, NK_ANTI_ALIASING_ON);

    // Final graphics command buffer submit
    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update,
        .window_width = 1024,
        .window_height = 760
    };
}
