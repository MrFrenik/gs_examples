/*================================================================
    * Copyright: 2020 John Jackson
    * lines 

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_VG_IMPL
#include <gs/util/gs_vg.h>

#define GLYPH_CACHE_SIZE  256

typedef struct 
{
    int32_t codepoint;
    int32_t advance;
    int32_t x1, y1, x2, y2;
} font_glyph_t;

typedef struct
{
    stbtt_fontinfo info;
    int32_t x1, y1, x2, y2;
    int32_t ascent;
    int32_t descent;
    int32_t linegap;
    struct {
        uint8_t* data;
        size_t sz;
    } memory;
    font_glyph_t cache[GLYPH_CACHE_SIZE];
} font_t;

font_t app_load_font_from_file(const char* path)
{
    font_t font = {0};
    font.memory.data = gs_platform_read_file_contents(path, "rb", &font.memory.sz);
    if (!stbtt_InitFont(&font.info, font.memory.data, 0)) return font;

    stbtt_GetFontVMetrics(&font.info, &font.ascent, &font.descent, &font.linegap);
    stbtt_GetFontBoundingBox(&font.info, &font.x1, &font.y1, &font.x2, &font.y2);
    memset(font.cache, 0, sizeof(font.cache));
    return font;
} 

// It will be faster to calculate all of this shape data up front, then just pull verts from glyphs as needed
void app_path_font_glyph(gs_vg_ctx_t* ctx, const font_t* font, int32_t codepoint, 
    gs_vec2 origin, float scl)
{
    int32_t index = stbtt_FindGlyphIndex(&font->info, codepoint);
    stbtt_vertex* v = NULL;
    int32_t num_verts = stbtt_GetGlyphShape(&font->info, index, &v);
    float x = origin.x;
    float y = origin.y;
    // scl = 1.f;
    for (int32_t i = 0; i < num_verts; ++i)
    {
        switch (v[i].type)
        {
            case STBTT_vmove:
            {
                gsvg_path_moveto(ctx, x + scl * v[i].x, y + scl * (1.f - v[i].y));
            } break;

            case STBTT_vline:
            {
                gsvg_path_lineto(ctx, x + scl * v[i].x, y + scl * (1.f - v[i].y));
            } break;

            case STBTT_vcurve:
            {
                gsvg_path_qbezierto(ctx, 
                        x + scl * v[i].cx, y + scl * (1.f - v[i].cy), 
                        x + scl * v[i].x, y + scl * (1.f - v[i].y));
            } break;

            case STBTT_vcubic:
            {
                gsvg_path_cbezierto(ctx, 
                        x + scl * v[i].cx, y + scl * (1.f - v[i].cy), 
                        x + scl * v[i].cx1, y + scl * (1.f - v[i].cy1), 
                        x + scl * v[i].x, y + scl * (1.f - v[i].y));
            } break;
        }
    }

    stbtt_FreeShape(&font->info, v);
}

static struct
{
    gs_command_buffer_t  cb;
    gs_immediate_draw_t  gsi; 
    gs_vg_ctx_t vg; 
    font_t font;
} app; 

void init()
{
    app.cb = gs_command_buffer_new(); 
    app.gsi = gs_immediate_draw_new(gs_platform_main_window()); 
    app.vg = gs_vg_ctx_new();
	const char* asset_dir = gs_platform_dir_exists("./assets") ? "./assets" : "../assets";
    gs_snprintfc(TMP, 256, "%s/liberation_mono_regular.ttf", asset_dir);
    gs_assert(gs_platform_file_exists(TMP));
    app.font = app_load_font_from_file(TMP);
}

void update()
{
    gs_command_buffer_t* cb = &app.cb;
    gs_immediate_draw_t* gsi = &app.gsi;
    gs_vg_ctx_t* vg = &app.vg;
    gs_vec2 ws = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    gs_vec2 mp = gs_platform_mouse_positionv();

    gs_vg_paint_t paint_blue = (gs_vg_paint_t) {
        .thickness = 3.f,
        .color = GS_COLOR_WHITE,
        .end = GS_VG_END_ROUND,
        .joint = GS_VG_JOINT_ROUND,
        .anti_alias = true,
        .aa_scale = 1.f
    };

    gs_vg_paint_t paint_red = (gs_vg_paint_t) {
        .thickness = 0.1f,
        .color = GS_COLOR_RED,
        .joint = GS_VG_JOINT_MITER,
        .anti_alias = true,
        .aa_scale = 0.5f
    };

    gs_vg_paint_t paint_bubble = (gs_vg_paint_t) {
        .thickness = 0.1f,
        .color = GS_COLOR_ORANGE,
        .joint = GS_VG_JOINT_MITER,
        .end = GS_VG_END_BUTT,
        .anti_alias = true,
        .aa_scale = 0.5f
    };

    static float thickness = 0.1f;
    static float aa_scale = 0.1f;
    if (gs_platform_key_down(GS_KEYCODE_UP)) thickness += 0.1f;
    if (gs_platform_key_down(GS_KEYCODE_DOWN)) thickness = gs_max(0.1f, thickness - 0.1f);
    if (gs_platform_key_down(GS_KEYCODE_E)) aa_scale += 0.1f;
    if (gs_platform_key_down(GS_KEYCODE_Q)) aa_scale = gs_max(0.1f, aa_scale - 0.1f);
    gs_vg_paint_t paint_font = (gs_vg_paint_t) {
        .thickness = thickness,
        .color = GS_COLOR_PURPLE,
        .joint = GS_VG_JOINT_ROUND,
        .end = GS_VG_END_JOINT,
        .anti_alias = true,
        .aa_scale = aa_scale
    };

    gs_vg_paint_t paint_green = (gs_vg_paint_t) {
        .thickness = 0.1f,
        .color = GS_COLOR_GREEN,
        .joint = GS_VG_JOINT_ROUND,
        .end = GS_VG_END_ROUND,
        .anti_alias = true,
        .aa_scale = 0.8f
    };

    gsvg_frame_begin(vg, (uint32_t)ws.x, (uint32_t)ws.y);

    gsvg_path_begin(vg);
    gsvg_paint(vg, paint_green);
	gsvg_path_moveto(vg, 100.f, 100.f);
	gsvg_path_lineto(vg, 150.f, 100.f);
	gsvg_path_lineto(vg, 150.f, 150.f);
	gsvg_path_lineto(vg, 250.f, 250.f);
	gsvg_path_lineto(vg, 150.f, 250.f);
	gsvg_path_close(vg);
    gsvg_path_end(vg, GS_VG_STROKE | GS_VG_FILL);

    /*
    gsvg_path_begin(vg);
    gsvg_paint(vg, paint_green);
    gsvg_path_arc(vg, 75.f, 75.f, 50.f, 0.f, 360.f); 
    gsvg_path_arc(vg, 75.f, 75.f, 30.f, 360.f - 45.f, 180.f);
    gsvg_path_arc(vg, 60.f, 65.f, 5.f, 0.f, 360.f);
    gsvg_path_arc(vg, 90.f, 65.f, 5.f, 0.f, 360.f);
    gsvg_path_stroke(vg);

    static float t = 0.f;
    t = fmodf(gs_subsystem(platform)->time.elapsed * 0.01f, 360.f);

    gsvg_path_begin(vg);
    gsvg_paint(vg, paint_bubble);
    gsvg_path_arc(vg, 250.f, 300.f, 150.f, 0.f, t);
    gsvg_path_stroke(vg);

    float min = 0.005f;
    float max = 0.2f;
    float tm = (sin(gs_subsystem(platform)->time.elapsed * 0.001f) * 0.5f + 0.5f) * max;
    float scl = gs_map_range(0.f, max, min, max, tm);
    static int32_t codepoint = 0;
    codepoint = (int32_t)(gs_subsystem(platform)->time.elapsed * 0.01f) % 256;
    codepoint = (int32_t)gs_map_range(0.f, 256.f, 33.f, 128.f, (float)codepoint);

    gs_mt_rand_t rand = gs_rand_seed(gs_hash_str64("vg"));
    for (uint32_t i = 0; i < 50; ++i)
    {
        float x = gs_rand_gen_range(&rand, 0.f, 800.f);
        float y = gs_rand_gen_range(&rand, 0.f, 600.f);
        gsvg_path_begin(vg);
        gsvg_paint(vg, paint_font);
        app_path_font_glyph(vg, &app.font, (codepoint + i) % 256, gs_v2(x, y), scl);
        gsvg_path_stroke(vg);
    }
    */
    
	gsvg_frame_end(vg); 

	gsvg_renderpass_submit(vg, cb, ws, gs_color(10, 10, 10, 255)); 

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






