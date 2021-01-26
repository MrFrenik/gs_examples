#ifndef __GS_NUKLEAR_H__
#define __GS_NUKLEAR_H__

#include <gs/gs.h>
#include <Nuklear/src/nuklear.h>

#ifndef GS_NK_TEXT_MAX
    #define GS_NK_TEXT_MAX 256
#endif

typedef enum gs_nk_init_state 
{
    GS_NK_DEFAULT = 0x00,
    GS_NK_INSTALL_CALLBACKS
} gs_nk_init_state;

typedef struct gs_nk_ctx_t 
{
    struct nk_context nk_ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    uint32_t text[GS_NK_TEXT_MAX];
    int32_t text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int32_t is_double_click_down;
    struct nk_vec2 double_click_pos;
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    gs_byte_buffer_t tmp_vertex_data;
    gs_byte_buffer_t tmp_index_data;
    gs_handle(gs_graphics_pipeline_t) pip;
    gs_handle(gs_graphics_buffer_t) vbo;
    gs_handle(gs_graphics_buffer_t) ibo;
    gs_handle(gs_graphics_shader_t) shader;
    gs_handle(gs_graphics_texture_t) font_tex; 
    gs_handle(gs_graphics_buffer_t) u_tex;
    gs_handle(gs_graphics_buffer_t) u_proj;
    uint32_t window_hndl;  
    int32_t width, height;
    int32_t display_width, display_height;
} gs_nk_ctx_t;

NK_API struct nk_context*   gs_nk_init(gs_nk_ctx_t* gs, uint32_t win_hndl, enum gs_nk_init_state init_state);
NK_API void                 gs_nk_render(gs_nk_ctx_t* gs, enum nk_anti_aliasing AA, int32_t max_vertex_buffer, int32_t max_element_buffer);

NK_INTERN void              gs_nk_clipboard_paste(nk_handle usr, struct nk_text_edit *edit);
NK_INTERN void              gs_nk_clipboard_copy(nk_handle usr, const char *text, int32_t len);

NK_API void                 gs_nk_device_create(gs_nk_ctx_t* gs);

/*===============================
//      Implementation
================================*/

#ifdef GS_NK_IMPL

#ifndef GS_NK_DOUBLE_CLICK_LO
    #define GS_NK_DOUBLE_CLICK_LO 0.02
#endif
#ifndef GS_NK_DOUBLE_CLICK_HI
    #define GS_NK_DOUBLE_CLICK_HI 0.2
#endif

typedef struct gs_nk_vertex_t 
{
    float position[2];
    float uv[2];
    nk_byte col[4];
} gs_nk_vertex_t;

#ifdef GS_PLATFORM_APPLE
  #define NK_SHADER_VERSION "#version 150\n"
#else
  #define NK_SHADER_VERSION "#version 300 es\n"
#endif

NK_API void
gs_nk_device_create(gs_nk_ctx_t* gs)
{
    static const char* nk_vertsrc =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";

    static const char* nk_fragsrc =
        NK_SHADER_VERSION
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    // Initialize commands
    nk_buffer_init_default(&gs->cmds);

    // Shader source description
    gs_graphics_shader_source_desc_t sources[] = {
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = nk_vertsrc},
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = nk_fragsrc}
    };

    // Create shader
    gs->shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = sources, 
            .size = sizeof(sources),
            .name = "nuklear_gui"
        }
    );

    // Sampler buffer desc
    gs_graphics_sampler_desc_t sdesc = {
        .type = GS_GRAPHICS_SAMPLER_2D
    };

    // Construct sampler buffer
    gs->u_tex = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_SAMPLER,
            .data = &sdesc,
            .size = sizeof(sdesc),
            .name = "Texture"
        }
    );

    gs->u_proj = gs_graphics_buffer_create (
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_UNIFORM,                                         // Type of buffer (uniform)
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4},    // Description of internal uniform data
            .size = sizeof(gs_graphics_uniform_desc_t),                                 // Size of uniform description (used for counts, if uniform block used)
            .name = "ProjMtx"                                                           // Name of uniform (used for linkage)
        }
    );

    // Construct vertex buffer
    gs->vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
            .data = NULL
        }
    );

    // Create index buffer
    gs->ibo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_INDEX,
            .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
            .data = NULL
        }
    );

    gs_graphics_vertex_attribute_type layout[] = {
        GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,        // Position
        GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2,        // UV
        GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4          // Color
    };

    // Create pipeline
    gs->pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = gs->shader,
                .index_buffer_element_size = sizeof(uint32_t)
            },
            .blend = {
                .func = GS_GRAPHICS_BLEND_EQUATION_ADD,
                .src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
                .dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA              
            },
            .layout = layout,
            .size = sizeof(layout)
        }
    );
}

NK_API struct nk_context* gs_nk_init(gs_nk_ctx_t* gs, uint32_t win_hndl, enum gs_nk_init_state init_state)
{
    // glfwSetWindowUserPointer(win, glfw);
    // glfw->win = win;
    // if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
    //     glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
    //     glfwSetCharCallback(win, nk_glfw3_char_callback);
    //     glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    // }
    gs->window_hndl = win_hndl;
    nk_init_default(&gs->nk_ctx, 0);
    gs->nk_ctx.clip.copy = gs_nk_clipboard_copy;
    gs->nk_ctx.clip.paste = gs_nk_clipboard_paste;
    gs->nk_ctx.clip.userdata = nk_handle_ptr(0);
    gs->last_button_click = 0;
    gs_nk_device_create(gs);
    gs->tmp_vertex_data = gs_byte_buffer_new();
    gs->tmp_index_data = gs_byte_buffer_new();
    gs->is_double_click_down = nk_false;
    gs->double_click_pos = nk_vec2(0, 0);

    return &gs->nk_ctx;
}

NK_API void
gs_nk_new_frame(gs_nk_ctx_t* gs)
{
    int i;
    double x, y;
    struct nk_context* ctx = &gs->nk_ctx;

    // Get window size
    gs_platform_window_size(gs->window_hndl, &gs->width, &gs->height);
    // Get frame buffer size
    gs_platform_framebuffer_size(gs->window_hndl, &gs->display_width, &gs->display_height);

    // Calculate fb scale
    gs->fb_scale.x = (float)gs->display_width/(float)gs->width;
    gs->fb_scale.y = (float)gs->display_height/(float)gs->height;

    nk_input_begin(ctx);
    for (i = 0; i < gs->text_len; ++i)
        nk_input_unicode(ctx, gs->text[i]);

// #ifdef NK_GLFW_GL3_MOUSE_GRABBING
//     /* optional grabbing behavior */
//     if (ctx->input.mouse.grab)
//         glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
//     else if (ctx->input.mouse.ungrab)
//         glfwSetInputMode(glfw->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
// #endif

    nk_input_key(ctx, NK_KEY_DEL, gs_platform_key_pressed(GS_KEYCODE_DELETE));
    nk_input_key(ctx, NK_KEY_ENTER, gs_platform_key_pressed(GS_KEYCODE_ENTER));
    nk_input_key(ctx, NK_KEY_TAB, gs_platform_key_pressed(GS_KEYCODE_TAB));
    nk_input_key(ctx, NK_KEY_BACKSPACE, gs_platform_key_pressed(GS_KEYCODE_BSPACE));
    nk_input_key(ctx, NK_KEY_UP, gs_platform_key_pressed(GS_KEYCODE_UP));
    nk_input_key(ctx, NK_KEY_DOWN, gs_platform_key_pressed(GS_KEYCODE_DOWN));
    nk_input_key(ctx, NK_KEY_TEXT_START, gs_platform_key_pressed(GS_KEYCODE_HOME));
    nk_input_key(ctx, NK_KEY_TEXT_END, gs_platform_key_pressed(GS_KEYCODE_END));
    nk_input_key(ctx, NK_KEY_SCROLL_START, gs_platform_key_pressed(GS_KEYCODE_HOME));
    nk_input_key(ctx, NK_KEY_SCROLL_END, gs_platform_key_pressed(GS_KEYCODE_END));
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, gs_platform_key_pressed(GS_KEYCODE_PGDOWN));
    nk_input_key(ctx, NK_KEY_SCROLL_UP, gs_platform_key_pressed(GS_KEYCODE_PGUP));
    nk_input_key(ctx, NK_KEY_SHIFT, gs_platform_key_pressed(GS_KEYCODE_LSHIFT)||
                                    gs_platform_key_pressed(GS_KEYCODE_RSHIFT));

    if (gs_platform_key_pressed(GS_KEYCODE_LCTRL) ||
        gs_platform_key_pressed(GS_KEYCODE_RCTRL)) {
        nk_input_key(ctx, NK_KEY_COPY, gs_platform_key_pressed(GS_KEYCODE_C));
        nk_input_key(ctx, NK_KEY_PASTE, gs_platform_key_pressed(GS_KEYCODE_V));
        nk_input_key(ctx, NK_KEY_CUT, gs_platform_key_pressed(GS_KEYCODE_X));
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, gs_platform_key_pressed(GS_KEYCODE_Z));
        nk_input_key(ctx, NK_KEY_TEXT_REDO, gs_platform_key_pressed(GS_KEYCODE_R));
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, gs_platform_key_pressed(GS_KEYCODE_LEFT));
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, gs_platform_key_pressed(GS_KEYCODE_RIGHT));
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, gs_platform_key_pressed(GS_KEYCODE_B));
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, gs_platform_key_pressed(GS_KEYCODE_E));
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, gs_platform_key_pressed(GS_KEYCODE_LEFT));
        nk_input_key(ctx, NK_KEY_RIGHT, gs_platform_key_pressed(GS_KEYCODE_RIGHT));
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    gs_platform_mouse_position((float*)&x, (float*)&y);

//     glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);

// #ifdef NK_GLFW_GL3_MOUSE_GRABBING
//     if (ctx->input.mouse.grabbed) {
//         glfwSetCursorPos(glfw->win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
//         ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
//         ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
//     }
// #endif
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, gs_platform_mouse_pressed(GS_MOUSE_LBUTTON));
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, gs_platform_mouse_pressed(GS_MOUSE_MBUTTON));
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, gs_platform_mouse_pressed(GS_MOUSE_RBUTTON));
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)gs->double_click_pos.x, (int)gs->double_click_pos.y, gs->is_double_click_down);
    nk_input_scroll(ctx, gs->scroll);
    nk_input_end(ctx);
    gs->text_len = 0;
    gs->scroll = nk_vec2(0,0);
}

NK_API void
gs_nk_render(gs_nk_ctx_t* gs, enum nk_anti_aliasing AA, int32_t max_vertex_buffer, int32_t max_element_buffer)
{
    struct nk_buffer vbuf, ebuf;
    float ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (float)gs->width;
    ortho[1][1] /= (float)gs->height;

    // Set up binds
    // Convert vertex/index data into buffers
    // Upload buffer data
    // Set up render pass
    // Set viewport
    // Bind pipeline
    // Bind bindings
    // Draw

    /* setup program */
    // glUseProgram(dev->prog);
    // glUniform1i(dev->uniform_tex, 0);
    // glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    // glViewport(0,0,(GLsizei)glfw->display_width,(GLsizei)glfw->display_height);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command* cmd;
        void *vertices, *elements;
        const nk_draw_index* offset = NULL;

        /* allocate vertex and element buffer */
        // glBindVertexArray(dev->vao);
        // glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        // glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        // vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        // elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            // struct nk_convert_config config;
            // static const struct nk_draw_vertex_layout_element vertex_layout[] = {
            //     {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
            //     {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
            //     {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
            //     {NK_VERTEX_LAYOUT_END}
            // };
            // NK_MEMSET(&config, 0, sizeof(config));
            // config.vertex_layout = vertex_layout;
            // config.vertex_size = sizeof(struct nk_glfw_vertex);
            // config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
            // config.null = dev->null;
            // config.circle_segment_count = 22;
            // config.curve_segment_count = 22;
            // config.arc_segment_count = 22;
            // config.global_alpha = 1.0f;
            // config.shape_AA = AA;
            // config.line_AA = AA;

            // /* setup buffers to load vertices and elements */
            // nk_buffer_init_fixed(&vbuf, vertices, (size_t)max_vertex_buffer);
            // nk_buffer_init_fixed(&ebuf, elements, (size_t)max_element_buffer);
            // nk_convert(&glfw->ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }
        // glUnmapBuffer(GL_ARRAY_BUFFER);
        // glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        // nk_draw_foreach(cmd, &glfw->ctx, &dev->cmds)
        {
            // if (!cmd->elem_count) continue;
            // glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            // glScissor(
            //     (GLint)(cmd->clip_rect.x * glfw->fb_scale.x),
            //     (GLint)((glfw->height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * glfw->fb_scale.y),
            //     (GLint)(cmd->clip_rect.w * glfw->fb_scale.x),
            //     (GLint)(cmd->clip_rect.h * glfw->fb_scale.y));
            // glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            // offset += cmd->elem_count;
        }
        // nk_clear(&glfw->ctx);
        // nk_buffer_clear(&dev->cmds);
    }

    /* default OpenGL state */
    // glUseProgram(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);
    // glDisable(GL_BLEND);
    // glDisable(GL_SCISSOR_TEST);
}

NK_INTERN void
gs_nk_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    // struct nk_glfw* glfw = usr.ptr;
    // const char *text = glfwGetClipboardString(glfw->win);
    // if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    // (void)usr;
}

NK_INTERN void
gs_nk_clipboard_copy(nk_handle usr, const char *text, int32_t len)
{
    // char *str = 0;
    // if (!len) return;
    // str = (char*)malloc((size_t)len+1);
    // if (!str) return;
    // memcpy(str, text, (size_t)len);
    // str[len] = '\0';
    // struct nk_glfw* glfw = usr.ptr;
    // glfwSetClipboardString(glfw->win, str);
    // free(str);
}

#undef GS_NK_IMPL

#endif // GS_NK_IMPL

#endif // __GS_NUKLEAR_H__