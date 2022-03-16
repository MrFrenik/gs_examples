#ifndef __GS_NUKLEAR_H__
#define __GS_NUKLEAR_H__

#ifndef GS_NK_TEXT_MAX
    #define GS_NK_TEXT_MAX 256
#endif

#define GS_NK_MAX_VERTEX_BUFFER 512 * 1024
#define GS_NK_MAX_INDEX_BUFFER  128 * 1024

typedef enum gs_nk_init_state 
{
    GS_NK_DEFAULT = 0x00,
    GS_NK_INSTALL_CALLBACKS
} gs_nk_init_state;

typedef struct gs_nk_ctx_t 
{
    struct nk_context nk_ctx;
    struct nk_font_atlas* atlas;
    struct nk_vec2 fb_scale;
    uint32_t text[GS_NK_TEXT_MAX];
    int32_t text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int32_t is_double_click_down;
    struct nk_vec2 double_click_pos;
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    void* tmp_vertex_data;
    void* tmp_index_data;
    gs_handle(gs_graphics_pipeline_t) pip;
    gs_handle(gs_graphics_vertex_buffer_t) vbo;
    gs_handle(gs_graphics_index_buffer_t) ibo;
    gs_handle(gs_graphics_shader_t) shader;
    gs_handle(gs_graphics_texture_t) font_tex; 
    gs_handle(gs_graphics_uniform_t) u_tex;
    gs_handle(gs_graphics_uniform_t) u_proj;
    uint32_t window_hndl;  
    int32_t width, height;
    int32_t display_width, display_height;
} gs_nk_ctx_t;

NK_API struct nk_context*   gs_nk_init(gs_nk_ctx_t* gs, uint32_t win_hndl, enum gs_nk_init_state init_state);
NK_API void                 gs_nk_new_frame(gs_nk_ctx_t* gs);
NK_API void                 gs_nk_render(gs_nk_ctx_t* gs, gs_command_buffer_t* cb, enum nk_anti_aliasing AA);
NK_API void                 gs_nk_device_upload_atlas(gs_nk_ctx_t* gs, const void *image, int32_t width, int32_t height);
NK_API void                 gs_nk_device_create(gs_nk_ctx_t* gs);

NK_API void gs_nk_font_stash_begin(struct gs_nk_ctx_t* gs, struct nk_font_atlas **atlas);
NK_API void gs_nk_font_stash_end(struct gs_nk_ctx_t* gs);

NK_INTERN void              gs_nk_clipboard_paste(nk_handle usr, struct nk_text_edit *edit);
NK_INTERN void              gs_nk_clipboard_copy(nk_handle usr, const char *text, int32_t len);

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

#ifdef GS_PLATFORM_WEB
    #define NK_SHADER_VERSION "#version 300 es\n"
#else
    #define NK_SHADER_VERSION "#version 330 core\n"
#endif

NK_API void
gs_nk_device_create(gs_nk_ctx_t* gs)
{
    static const char* nk_vertsrc =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 TexCoord;\n"
        "layout (location = 2) in vec4 Color;\n"
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

    // Construct sampler buffer
    gs->u_tex = gs_graphics_uniform_create(
        &(gs_graphics_uniform_desc_t) {
            .name = "Texture",
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_SAMPLER2D}
        }
    );

    gs->u_proj = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "ProjMtx",                                                           // Name of uniform (used for linkage)
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4}
        }
    );

    // Construct vertex buffer
    gs->vbo = gs_graphics_vertex_buffer_create(
        &(gs_graphics_index_buffer_desc_t) {
            .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
            .data = NULL
        }
    );

    // Create index buffer
    gs->ibo = gs_graphics_index_buffer_create(
        &(gs_graphics_index_buffer_desc_t) {
            .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
            .data = NULL
        }
    );

    // Vertex attr layout
    gs_graphics_vertex_attribute_desc_t gsnk_vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "Position"},  // Position
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2, .name = "TexCoord"},  // UV
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4, .name = "Color"}       // Color
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
            .layout = {
                .attrs = gsnk_vattrs,
                .size = sizeof(gsnk_vattrs)
            }
        }
    );
}

NK_API struct nk_context* gs_nk_init(gs_nk_ctx_t* gs, uint32_t win_hndl, enum gs_nk_init_state init_state)
{
    gs->window_hndl = win_hndl;
    nk_init_default(&gs->nk_ctx, 0);
    gs->nk_ctx.clip.copy = gs_nk_clipboard_copy;
    gs->nk_ctx.clip.paste = gs_nk_clipboard_paste;
    gs->nk_ctx.clip.userdata = nk_handle_ptr(0);
    gs->last_button_click = 0;
    gs_nk_device_create(gs);
    gs->is_double_click_down = nk_false;
    gs->double_click_pos = nk_vec2(0, 0);

    // Tmp data buffers for upload
    gs->tmp_vertex_data = gs_malloc(GS_NK_MAX_VERTEX_BUFFER);
    gs->tmp_index_data = gs_malloc(GS_NK_MAX_INDEX_BUFFER);

    gs_assert(gs->tmp_vertex_data);
    gs_assert(gs->tmp_index_data);

    // Font atlas
    gs->atlas = gs_malloc(sizeof(struct nk_font_atlas));

    return &gs->nk_ctx;
}

NK_API void
gs_nk_device_upload_atlas(gs_nk_ctx_t* gs, const void *image, int32_t width, int32_t height)
{
    // Create font texture
    gs->font_tex = gs_graphics_texture_create(
        &(gs_graphics_texture_desc_t){
            .width = width,
            .height = height,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR, 
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_LINEAR, 
            .data = (void*)image
        }
    );
}

NK_API void
gs_nk_font_stash_begin(struct gs_nk_ctx_t* gs, struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(gs->atlas);
    nk_font_atlas_begin(gs->atlas);
    if (atlas) {
        *atlas = gs->atlas;
    }
}

NK_API void
gs_nk_font_stash_end(struct gs_nk_ctx_t* gs)
{
    const void *image; int32_t w, h;
    image = nk_font_atlas_bake(gs->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);

    // Upload texture (this is where we'll set the font texture resource handle)
    gs_nk_device_upload_atlas(gs, image, w, h);

    // Create nk handle from resource handle
    nk_handle hndl = nk_handle_id((int32_t)gs->font_tex.id);

    nk_font_atlas_end(gs->atlas, hndl, &gs->null);

    if (gs->atlas->default_font)
        nk_style_set_font(&gs->nk_ctx, &gs->atlas->default_font->handle);
}

NK_API void
gs_nk_new_frame(gs_nk_ctx_t* gs)
{
    int i;
    int32_t x, y;
    struct nk_context* ctx = &gs->nk_ctx;

    // Cache platform pointer
    gs_platform_t* platform = gs_subsystem(platform);

    // Get window size
    gs_platform_window_size(gs->window_hndl, &gs->width, &gs->height);
    // Get frame buffer size
    gs_platform_framebuffer_size(gs->window_hndl, &gs->display_width, &gs->display_height);

    // Reset wheel
    gs->scroll.x = 0.f;
    gs->scroll.y = 0.f; 

    // Calculate fb scale
    gs->fb_scale.x = (float)gs->display_width/(float)gs->width;
    gs->fb_scale.y = (float)gs->display_height/(float)gs->height;

    nk_input_begin(ctx);
    {
        // Poll all events that occured this frame
        gs_platform_event_t evt = gs_default_val();
        while(gs_platform_poll_events(&evt, true))
        {
            switch(evt.type)
            {
                case GS_PLATFORM_EVENT_KEY:
                {
                    switch (evt.key.action)
                    {
                        case GS_PLATFORM_KEY_PRESSED:
                        {
                            if (gs->text_len < GS_NK_TEXT_MAX)
                                gs->text[gs->text_len++] = evt.key.codepoint;

                        } break;

                        default: break;
                    }
                } break;

                case GS_PLATFORM_EVENT_MOUSE:
                {
                    switch (evt.mouse.action)
                    {
                        case GS_PLATFORM_MOUSE_WHEEL:
                        {
                            gs->scroll.x = evt.mouse.wheel.x;
                            gs->scroll.y = evt.mouse.wheel.y;
                        } break;

                        case GS_PLATFORM_MOUSE_MOVE:
                        {
                            nk_input_motion(ctx, (int32_t)evt.mouse.move.x, (int32_t)evt.mouse.move.y);
                        } break;

                        default: break;
                    }
                } break;

                default: break;
            }
        }

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
        nk_input_key(ctx, NK_KEY_BACKSPACE, gs_platform_key_pressed(GS_KEYCODE_BACKSPACE));
        nk_input_key(ctx, NK_KEY_UP, gs_platform_key_pressed(GS_KEYCODE_UP));
        nk_input_key(ctx, NK_KEY_DOWN, gs_platform_key_pressed(GS_KEYCODE_DOWN));
        nk_input_key(ctx, NK_KEY_TEXT_START, gs_platform_key_pressed(GS_KEYCODE_HOME));
        nk_input_key(ctx, NK_KEY_TEXT_END, gs_platform_key_pressed(GS_KEYCODE_END));
        nk_input_key(ctx, NK_KEY_SCROLL_START, gs_platform_key_pressed(GS_KEYCODE_HOME));
        nk_input_key(ctx, NK_KEY_SCROLL_END, gs_platform_key_pressed(GS_KEYCODE_END));
        nk_input_key(ctx, NK_KEY_SCROLL_DOWN, gs_platform_key_pressed(GS_KEYCODE_PAGE_DOWN));
        nk_input_key(ctx, NK_KEY_SCROLL_UP, gs_platform_key_pressed(GS_KEYCODE_PAGE_UP));
        nk_input_key(ctx, NK_KEY_SHIFT, gs_platform_key_pressed(GS_KEYCODE_LEFT_SHIFT)||
                                        gs_platform_key_pressed(GS_KEYCODE_RIGHT_SHIFT));

        if (gs_platform_key_down(GS_KEYCODE_LEFT_CONTROL) ||
            gs_platform_key_down(GS_KEYCODE_RIGHT_CONTROL)) {
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

        gs_platform_mouse_position(&x, &y);
        nk_input_motion(ctx, (int32_t)x, (int32_t)y);

    #ifdef GS_NK_MOUSE_GRABBING
        if (ctx->input.mouse.grabbed) {
            // glfwSetCursorPos(glfw->win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
            ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
            ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
        }
    #endif

        // Should swap this over to polling events instead.
        nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, gs_platform_mouse_down(GS_MOUSE_LBUTTON));
        nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, gs_platform_mouse_down(GS_MOUSE_MBUTTON));
        nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, gs_platform_mouse_down(GS_MOUSE_RBUTTON));
        nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)gs->double_click_pos.x, (int)gs->double_click_pos.y, gs->is_double_click_down);
        nk_input_scroll(ctx, gs->scroll);
    }
    nk_input_end(ctx);

    gs->text_len = 0;
}

NK_API void
gs_nk_render(gs_nk_ctx_t* gs, gs_command_buffer_t* cb, enum nk_anti_aliasing AA)
{
    struct nk_buffer vbuf, ibuf;

    // Projection matrix
    float ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (float)gs->width;
    ortho[1][1] /= (float)gs->height;
    gs_mat4 m = gs_mat4_elem((float*)ortho);

    // Set up data binds
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = gs->vbo}},
        .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = gs->ibo}},
        .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = gs->u_proj, .data = &m}}
    };

    gs_assert(gs->tmp_vertex_data);
    gs_assert(gs->tmp_index_data);

    // Convert from command queue into draw list and draw to screen
    {
        const struct nk_draw_command* cmd;
        void *vertices, *indices;
        const nk_draw_index *offset = 0;

        vertices = gs->tmp_vertex_data;
        indices = gs->tmp_index_data;

        // Convert commands into draw lists
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct gs_nk_vertex_t, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct gs_nk_vertex_t, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct gs_nk_vertex_t, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct gs_nk_vertex_t);
            config.vertex_alignment = NK_ALIGNOF(struct gs_nk_vertex_t);
            config.null = gs->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            // Setup buffers to load vertices and indices
            nk_buffer_init_fixed(&vbuf, vertices, (size_t)GS_NK_MAX_VERTEX_BUFFER);
            nk_buffer_init_fixed(&ibuf, indices, (size_t)GS_NK_MAX_INDEX_BUFFER);
            nk_convert(&gs->nk_ctx, &gs->cmds, &vbuf, &ibuf, &config);
        }

        // Request update vertex data
        gs_graphics_vertex_buffer_request_update(cb, gs->vbo,
            &(gs_graphics_vertex_buffer_desc_t){
                .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
                .data = vertices,
                .size = GS_NK_MAX_VERTEX_BUFFER
            }
        );

        // Request update index data
        gs_graphics_index_buffer_request_update(cb, gs->ibo,
            &(gs_graphics_index_buffer_desc_t){
                .usage = GS_GRAPHICS_BUFFER_USAGE_STREAM,
                .data = indices,
                .size = GS_NK_MAX_INDEX_BUFFER
            }
        );

        // Render pass action for clearing the screen
        gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.0f, 0.0f, 0.0f, 1.f}};

        // Render pass
        gs_graphics_renderpass_begin(cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);

            // Bind pipeline for nuklear
            gs_graphics_pipeline_bind(cb, gs->pip);

            // Set viewport
            gs_graphics_set_viewport(cb, 0, 0, (uint32_t)gs->display_width, (uint32_t)gs->display_height);

            // Clear
            gs_graphics_clear(cb, &clear);

            // Global bindings for pipeline
            gs_graphics_apply_bindings(cb, &binds);

            // Iterate and draw all commands
            nk_draw_foreach(cmd, &gs->nk_ctx, &gs->cmds)
            {
                if (!cmd->elem_count) continue;

                // Grab handle from command texture id
                gs_handle(gs_graphics_texture_t) tex = gs_handle_create(gs_graphics_texture_t, cmd->texture.id);

                // Bind texture
                gs_graphics_bind_desc_t sbind = {
                    .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = gs->u_tex, .data = &tex, .binding = 0}}
                };

                // Bind individual texture binding
                gs_graphics_apply_bindings(cb, &sbind);

                // Set view scissor
                gs_graphics_set_view_scissor(cb, 
                   (uint32_t)(cmd->clip_rect.x * gs->fb_scale.x), 
                   (uint32_t)((gs->height - (cmd->clip_rect.y + cmd->clip_rect.h)) * gs->fb_scale.y),
                   (uint32_t)(cmd->clip_rect.w * gs->fb_scale.x),
                   (uint32_t)(cmd->clip_rect.h * gs->fb_scale.y)
                );

                // Draw elements
                gs_graphics_draw(cb, &(gs_graphics_draw_desc_t){.start = (size_t)offset, .count = (uint32_t)cmd->elem_count});

                // Increment offset for commands
                offset += cmd->elem_count;
            }

        gs_graphics_renderpass_end(cb);

        // Clear nuklear info
        nk_clear(&gs->nk_ctx);
        nk_buffer_clear(&gs->cmds);
    }
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
