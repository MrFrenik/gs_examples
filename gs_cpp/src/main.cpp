#include "app.h"

#define WIDTH 800
#define HEIGHT 600

// Custom asset definition
typedef struct custom_asset_t
{
    const char* name;
    uint32_t udata;
    float fdata;
} custom_asset_t;

// Custom load from file function you can use to load resource data from file
// Can give any optional parameters AFTER void* out parameter.
void load_custom_asset_from_file(const char* path, void* out, const char* name, uint32_t optional_uint_param, double optional_float_param)
{
    // Load your data here however you want...
    custom_asset_t* ca = (custom_asset_t*)out;

    // Set data for optional param
    ca->name = name;
    ca->udata = optional_uint_param;
    ca->fdata = (float)optional_float_param;
}

typedef struct sprite_t {
    uint32_t width;
    uint32_t height;
    gs_vec4 uvs;            // UVs for atlas
} sprite_t;

typedef struct sprite_animation_t
{
    gs_asset_t atlas;
    gs_dyn_array(sprite_t) frames;
    float time;
    float frame_rate;
    uint32_t cur_frame;
} sprite_animation_t;

sprite_animation_t create_sprite_animation(gs_asset_manager_t* gam, gs_asset_t atlas, gs_vec4* frames, size_t sz, float rate = 1.f)
{
    sprite_animation_t sa = {0};

    gs_asset_texture_t* tp = gs_assets_getp(gam, gs_asset_texture_t, atlas);
    uint32_t width = tp->desc.width;
    uint32_t height = tp->desc.height;

    // Iterate through frames
    uint32_t ct = sz / sizeof(gs_vec4);
    for (uint32_t i = 0; i < ct; ++i)
    {
        sprite_t sprite = {0};

        for (uint32_t j = 0; j < 4; ++j)
        {
            float v = frames[i].xyzw[j];
            sprite.uvs.xyzw[j] = j % 2 == 0 ? (float)((double)v / (double)width) : (float)((double)v / (double)height);
            sprite.width = j == 2 ? j : sprite.width;
            sprite.height = j == 3 ? j : sprite.height;
        }

        // Correct uvs (calculate bounds)
        sprite.uvs.z = sprite.uvs.x + sprite.uvs.z;
        sprite.uvs.w = (sprite.uvs.w + sprite.uvs.y);
        sprite.uvs.y = sprite.uvs.y;

        // Push back frame
        gs_dyn_array_push(sa.frames, sprite);
    }

    sa.atlas = atlas;
    sa.frame_rate = rate;

    return sa;
}

void sprite_animation_tick(sprite_animation_t* anim, float delta)
{
    anim->time += delta;
    if (anim->time >= anim->frame_rate)
    {
        anim->cur_frame = (anim->cur_frame + 1) % gs_dyn_array_size(anim->frames);
        anim->time = 0.f;
    }
}

// Custom derived App class
class MyApp : public App
{
    public:

        MyApp() 
        {
        }

        virtual ~MyApp() override {}

        void LoadCustomData(const std::string& name, const std::string& path, const char* opt_name = "default", uint32_t udata = 10, float fdata = 10.f)
        {
            PlaceAsset(name, gs_assets_load_from_file(&m_gsa, custom_asset_t, NULL, opt_name, udata, fdata)); 
        }

        virtual void Init() override 
        {
            // Register custom importer type
            gs_asset_importer_desc_t adesc = {};
            adesc.load_from_file = (gs_asset_load_func)load_custom_asset_from_file;
            gs_assets_register_importer(&m_gsa, custom_asset_t, &adesc);

            // Register sprite animation importer type
            adesc = {};
            gs_assets_register_importer(&m_gsa, sprite_animation_t, &adesc);

            // Create asset and get handle for custom data placed into asset manager
            custom_asset_t custom = {};
            custom.name = "whatever";
            custom.udata = 50;
            custom.fdata = 3.145f;
            PlaceAsset("c0", gs_assets_create_asset(&m_gsa, custom_asset_t, &custom));

            // Load custom data "from file" (can provide optional data AFTER path)
            LoadCustomData("c1", "path/to/data", "name", 10, 2.45f);

            // Load a texture with custom parameters
            gs_graphics_texture_desc_t desc = {};
            desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
            desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
            desc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
            LoadTexture("atlas", "./assets/atlas.png", &desc);

            // Load a texture with default parameters
            LoadTexture("tx", "./assets/tx.png");

            // Load font with default font point (16)
            LoadFont("font_16", "./assets/font.ttf");

            // Load font with custom font point
            LoadFont("font_64", "./assets/font.ttf", 64);

            // Create custom sprite animation using atlas
            gs_asset_t atlas = GetAsset("atlas");

            // Frame uv, height, width information for idle animation
            gs_vec4 idle_frames[] = {
                gs_v4(1, 1, 19, 32),
                gs_v4(22, 1, 19, 32),
                gs_v4(43, 1, 19, 32),
                gs_v4(64, 1, 19, 32),
                gs_v4(1, 36, 19, 32),
                gs_v4(22, 36, 19, 32),
                gs_v4(43, 36, 19, 32),
                gs_v4(64, 36, 19, 32),
                gs_v4(85, 1, 19, 32),
                gs_v4(85, 36, 19, 32),
                gs_v4(85, 71, 19, 32),
                gs_v4(106, 1, 19, 32)
            };

            sprite_animation_t anim = create_sprite_animation(&m_gsa, atlas, idle_frames, sizeof(idle_frames));
            PlaceAsset("idle", gs_assets_create_asset(&m_gsa, sprite_animation_t, &anim));
        }

        virtual void Update() override 
        {
            if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

            gs_asset_texture_t* tp0 = gs_assets_getp(&m_gsa, gs_asset_texture_t, GetAsset("atlas"));
            gs_asset_texture_t* tp1 = gs_assets_getp(&m_gsa, gs_asset_texture_t, GetAsset("tx"));
            gs_asset_font_t* fp0 = gs_assets_getp(&m_gsa, gs_asset_font_t, GetAsset("font_16"));
            gs_asset_font_t* fp1 = gs_assets_getp(&m_gsa, gs_asset_font_t, GetAsset("font_64"));
            custom_asset_t* c0 = gs_assets_getp(&m_gsa, custom_asset_t, GetAsset("c0"));
            custom_asset_t* c1 = gs_assets_getp(&m_gsa, custom_asset_t, GetAsset("c1"));
            sprite_animation_t* ap = gs_assets_getp(&m_gsa, sprite_animation_t, GetAsset("idle"));

            // Increment sprite animation
            sprite_animation_tick(ap, 0.1f);

            // 2D sprite rendering (animation)
            gsi_push_matrix(&m_gsi, GSI_MATRIX_PROJECTION);
            {
                gsi_camera2D(&m_gsi);
                gsi_blend_enabled(&m_gsi, true);
                gsi_push_matrix(&m_gsi, GSI_MATRIX_MODELVIEW);
                {
                    // Get uvs for sprite animation
                    sprite_t* frame = &ap->frames[ap->cur_frame];
                    gs_vec4* uvp = &frame->uvs;

                    float uvx0 = uvp->x;
                    float uvy0 = uvp->y;
                    float uvx1 = uvp->z;
                    float uvy1 = uvp->w;

                    gs_vec2 dim = gs_vec2_scale(gs_v2(frame->width, frame->height), 80.f);
                    gs_vec2 a = gs_v2(100.f, 100.f);
                    gs_vec2 b = gs_v2(a.x + dim.x, a.y + dim.y);

                    gsi_texture(&m_gsi, tp0->hndl);
                    gsi_rectvx(&m_gsi, a, b, gs_v2(uvx0, uvy0), gs_v2(uvx1, uvy1), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
                gsi_pop_matrix(&m_gsi); // modelview
            }
            gsi_pop_matrix(&m_gsi); // projection

            gsi_defaults(&m_gsi);
            
            // Rotating cube
            gsi_push_matrix(&m_gsi, GSI_MATRIX_PROJECTION);
            {
                gsi_camera3D(&m_gsi);
                gsi_depth_enabled(&m_gsi, true);
                gsi_face_cull_enabled(&m_gsi, true);
                gsi_push_matrix(&m_gsi, GSI_MATRIX_MODELVIEW);
                {
                    gsi_transf(&m_gsi, 2.f, 3.f, -10.f);
                    gsi_scalef(&m_gsi, 5.f, 5.f, 5.f);
                    gsi_rotatefv(&m_gsi, gs_platform_elapsed_time() * 0.001f, GS_ZAXIS);
                    gsi_rotatefv(&m_gsi, gs_platform_elapsed_time() * 0.002f, GS_YAXIS);
                    gsi_rotatefv(&m_gsi, gs_platform_elapsed_time() * 0.004f, GS_XAXIS);
                    gsi_box(&m_gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_LINES);
                    gsi_texture(&m_gsi, tp1->hndl);
                    gsi_box(&m_gsi, 0.f, 0.f, 0.f, 0.5f, 0.5f, 0.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
                }
                gsi_pop_matrix(&m_gsi); // modelview
            }
            gsi_pop_matrix(&m_gsi); // projection

            // Reset defaults
            gsi_defaults(&m_gsi);

            // Metrics for custom types with custom font rendering
            gsi_push_matrix(&m_gsi, GSI_MATRIX_PROJECTION);
            {
                gsi_camera2D(&m_gsi);

                gs_snprintfc(tmpbuf, 512, "c0 <%zu, %.2f, %s>", c0->udata, c0->fdata, c0->name);
                gsi_text(&m_gsi, 200.f, 400.f, tmpbuf, fp0, false, 255, 200, 100, 255);

                gs_snprintfc(tmpbuf1, 256, "c1 <%zu, %.2f, %s>", c1->udata, c1->fdata, c1->name);
                gsi_text(&m_gsi, 50.f, 500.f, tmpbuf1, fp1, false, 255, 0, 100, 255);
            }
            gsi_pop_matrix(&m_gsi); // projection

            gsi_render_pass_submit(&m_gsi, &m_gcb, gs_color(20, 20, 20, 255));
            gs_graphics_submit_command_buffer(&m_gcb);
        }

        virtual void Shutdown() override 
        {
            gs_println("Shutdown...");
        }
};

int main(int argc, char *argv[]) 
{
    MyApp app;
    app.Run();

    return 0;
}