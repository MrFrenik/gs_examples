#include "app.h"

#define WIDTH 800
#define HEIGHT 600

class MyApp : public App
{
    public:

        MyApp() 
        {
        }

        virtual ~MyApp() override {}

        virtual void Init() override 
        {
            // Load a texture
            gs_graphics_texture_desc_t desc = {};
            desc.format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA8;
            desc.min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
            desc.mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST;
            LoadTexture("atlas", "./assets/atlas.png", &desc);
            
            LoadTexture("tx", "./assets/tx.png");
        }

        virtual void Update() override 
        {
            if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

            gs_asset_texture_t* tp0 = gs_assets_getp(&m_gsa, gs_asset_texture_t, m_assetTable["atlas"]);
            gs_asset_texture_t* tp1 = gs_assets_getp(&m_gsa, gs_asset_texture_t, m_assetTable["tx"]);

            gsi_camera2D(&m_gsi);
            gsi_blend_enabled(&m_gsi, true);
            gsi_push_matrix(&m_gsi, GSI_MATRIX_MODELVIEW);
            {
                gsi_transf(&m_gsi, 100.f, 100.f, 0.f);
                gsi_rotatefv(&m_gsi, gs_platform_elapsed_time() * 0.001f, GS_ZAXIS);
                gsi_scalef(&m_gsi, 400.f, 400.f, 1.f);
                gsi_texture(&m_gsi, tp0->hndl);
                gsi_rectvd(&m_gsi, gs_v2(-0.5f, -0.5f), gs_v2(0.5f, 0.5f), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_color(255, 255, 255, 200), GS_GRAPHICS_PRIMITIVE_TRIANGLES);
            }
            gsi_pop_matrix(&m_gsi);
            gsi_texture(&m_gsi, tp1->hndl);
            gsi_rectvd(&m_gsi, gs_v2(200.f, 200.f), gs_v2(200.f, 200.f), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), gs_color(255, 255, 255, 200), GS_GRAPHICS_PRIMITIVE_TRIANGLES);
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