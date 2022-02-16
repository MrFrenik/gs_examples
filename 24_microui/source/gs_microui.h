//
// Created by Sushil on 2021/07/10.
//

#ifndef GS_EXAMPLES_GS_MICROUI_H
#define GS_EXAMPLES_GS_MICROUI_H
#include <microui/atlas.inl>

/*== micrui renderer =========================================================*/
typedef struct gs_mu_ctx{
    mu_Context mu;
    gs_immediate_draw_t gsi;
    gs_handle(gs_graphics_texture_t) atlas_tex;

}gs_mu_ctx;
static int __text_width(mu_Font font, const char *text, int len) {
    int res = 0;
    for (const char* p = text; *p && len--; p++) {
        res += atlas[ATLAS_FONT + (unsigned char)*p].w;
    }
    return res;
}
static gs_mu_ctx * MU;
void mu_char_callback(void* handle, uint32_t codepoint)
{
    uint8_t txt[2] ={codepoint ,0};
    mu_input_text(&MU->mu, txt);
}
static int __text_height(mu_Font font) {
    return 18;
}
static void gs_mu_init(gs_mu_ctx * ctx)
{

    ctx->gsi = gs_immediate_draw_new(gs_platform_main_window());
    mu_init(&ctx->mu);
    ctx->mu.text_width = __text_width;
    ctx->mu.text_height = __text_height;
    uint32_t rgba8_size = ATLAS_WIDTH * ATLAS_HEIGHT * 4;
    uint32_t* rgba8_pixels = (uint32_t*) malloc(rgba8_size);
    for (int y = 0; y < ATLAS_HEIGHT; y++) {
        for (int x = 0; x < ATLAS_WIDTH; x++) {
            int index = y*ATLAS_WIDTH + x;
            rgba8_pixels[index] = 0x00FFFFFF | ((uint32_t)atlas_texture[index]<<24);
        }
    }
    ctx->atlas_tex  = gs_graphics_texture_create (
            &(gs_graphics_texture_desc_t) {
                .width = ATLAS_WIDTH,
                .height = ATLAS_HEIGHT,
                .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
                .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
                .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
                .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
                .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
                .data = rgba8_pixels
            }
    );

    MU = ctx;
    #ifndef __EMSCRIPTEN__
        gs_platform_set_character_callback(gs_platform_main_window() , mu_char_callback);
    #endif
}

void  gs_mu_new_frame(gs_mu_ctx * ctx){
    gs_vec2 mouse_pos = gs_platform_mouse_positionv();
    gs_platform_event_t evt = gs_default_val();
        while (gs_platform_poll_events(&evt, false))
        {
            switch (evt.type)
            {
                case GS_PLATFORM_EVENT_MOUSE:
                {
                    switch (evt.mouse.action)
                    {
                        case GS_PLATFORM_MOUSE_MOVE:
                        {
                            mu_input_mousemove(&ctx->mu,(int)evt.mouse.move.x,(int)evt.mouse.move.y);
                        } break;

                        case GS_PLATFORM_MOUSE_WHEEL:
                        {
                            mu_input_scroll(&ctx->mu, 0, (int)evt.mouse.wheel.y* -30);
                        } break;

                        case GS_PLATFORM_MOUSE_BUTTON_PRESSED:
                        {
                            int code = 1<<evt.mouse.button;
                            mu_input_mousedown(&ctx->mu,(int) mouse_pos.x,(int) mouse_pos.y,code);
                        } break;

                        case GS_PLATFORM_MOUSE_BUTTON_RELEASED:
                        {
                            int code = 1<<evt.mouse.button;
                            mu_input_mouseup(&ctx->mu,(int) mouse_pos.x,(int) mouse_pos.y,code);
                        } break;

                        case GS_PLATFORM_MOUSE_BUTTON_DOWN:
                        {
                            int code = 1<<evt.mouse.button;
                            mu_input_mousedown(&ctx->mu,(int) mouse_pos.x,(int) mouse_pos.y,code);
                        } break;

                        case GS_PLATFORM_MOUSE_ENTER:
                        {
                            // If there are user callbacks, could trigger them here
                        } break;

                        case GS_PLATFORM_MOUSE_LEAVE:
                        {
                            // If there are user callbacks, could trigger them here
                        } break;
                    }

                } break;

                case GS_PLATFORM_EVENT_KEY:
                {
                    switch (evt.key.action)
                    {

                        case GS_PLATFORM_KEY_PRESSED:
                        {
                            // Handled by char callback on desktop 
                             #ifdef __EMSCRIPTEN__
                                char txt[2] = {(char)evt.key.codepoint,0};
                                mu_input_text(&ctx->mu,txt);
                            #endif
                        } break;
                        case GS_PLATFORM_KEY_DOWN:
                        {
                            mu_input_keydown(&ctx->mu,evt.key.codepoint);
                        } break;

                        case GS_PLATFORM_KEY_RELEASED:
                        {
                            mu_input_keyup(&ctx->mu,evt.key.codepoint);
                        } break;
                    }

                } break;

                case GS_PLATFORM_EVENT_WINDOW:
                {
                    switch (evt.window.action)
                    {
                        default: break;
                    }

                } break;


                default: break;
            }
        }
}
void __push_quad(gs_immediate_draw_t * gsi,mu_Rect dst, mu_Rect src, mu_Color color) {
    float u0 = (float) src.x / (float) ATLAS_WIDTH;
    float v0 = (float) src.y / (float) ATLAS_HEIGHT;
    float u1 = (float) (src.x + src.w) / (float) ATLAS_WIDTH;
    float v1 = (float) (src.y + src.h) / (float) ATLAS_HEIGHT;

    float x0 = (float) dst.x;
    float y0 = (float) dst.y;
    float x1 = (float) (dst.x + dst.w);
    float y1 = (float) (dst.y + dst.h);
    gsi_begin(gsi, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_c4ub(gsi,color.r, color.g, color.b, color.a);

    // First triangle
    gsi_tc2f(gsi, u0, v0); gsi_v2f(gsi, x0, y0);
    gsi_tc2f(gsi, u1, v0); gsi_v2f(gsi, x1, y0);
    gsi_tc2f(gsi, u0, v1); gsi_v2f(gsi, x0, y1);

    // Second triangle
    gsi_tc2f(gsi, u1, v0); gsi_v2f(gsi, x1, y0);
    gsi_tc2f(gsi, u1, v1); gsi_v2f(gsi, x1, y1);
    gsi_tc2f(gsi, u0, v1); gsi_v2f(gsi, x0, y1);
    gsi_end(gsi);

}
void gs_mu_render(gs_mu_ctx * ctx,gs_command_buffer_t * cb)
{ 
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
    gsi_texture(&ctx->gsi,ctx->atlas_tex);
    gsi_camera2D(&ctx->gsi, fbs.x, fbs.y);
    mu_Command *cmd = NULL;
    while (mu_next_command(&ctx->mu, &cmd)) {
      switch (cmd->type) {
        case MU_COMMAND_TEXT:
        {
            mu_Rect dst =(mu_Rect) { .x = cmd->text.pos.x, .y =cmd->text.pos.y,.h = 0, .w = 0 };
            for (const char *p = cmd->text.str; *p; p++) {
                if ((*p & 0xc0) == 0x80) { continue; }
                int chr = mu_min((unsigned char) *p, 127);
                mu_Rect src = atlas[ATLAS_FONT + chr];
                dst.w = src.w;
                dst.h = src.h;
                __push_quad(&ctx->gsi,dst, src, cmd->text.color);
                dst.x += dst.w;
            }
        } break;
        case MU_COMMAND_RECT:

            __push_quad(&ctx->gsi,cmd->rect.rect, atlas[ATLAS_WHITE], cmd->rect.color);

            break;
        case MU_COMMAND_ICON:
            gsi_texture(&ctx->gsi,ctx->atlas_tex);
                mu_Rect src = atlas[cmd->icon.id];
                int x = cmd->icon.rect.x + (cmd->icon.rect.w - src.w) / 2;
                int y = cmd->icon.rect.y + (cmd->icon.rect.h - src.h) / 2;
                __push_quad(&ctx->gsi,mu_rect(x, y, src.w, src.h), src, cmd->icon.color);
            break;
        case MU_COMMAND_CLIP:
            gs_graphics_set_view_scissor(cb,cmd->clip.rect.x,cmd->clip.rect.y,cmd->clip.rect.w,cmd->clip.rect.h);
            break;
      }
    }
    gsi_draw(&ctx->gsi, cb);

}

#endif //GS_EXAMPLES_GS_MICROUI_H
