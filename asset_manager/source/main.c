/*================================================================
    * Copyright: 2020 John Jackson
    * asset_manager

    The purpose of this example is to demonstrate how to use the asset 
    manager util for creating custom importers, loading custom 
    resources from file and placing them into the asset subsystem.

    Included: 
        * Loading assets from included, registered importers:
            - gs_asset_texture_t
            - gs_asset_font_t
            - gs_asset_audio_t
        * Registering custom asset importer
        * Creating custom asset data
        * Getting raw asset data from asset manager using asset handles

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

gs_command_buffer_t gcb = {0}; 
gs_immediate_draw_t gsi = {0};
gs_asset_manager_t  gsa = {0};

// Custom asset definition
typedef struct custom_asset_t
{
    const char* name;
    uint32_t udata;
    float fdata;
} custom_asset_t;

// Custom load from file function you can use to load resource data from file
// Can give any optional parameters AFTER void* out parameter.
void load_custom_asset_from_file(const char* path, void* out, uint32_t optional_uint_param, double optional_float_param)
{
    // Load your data here however you want...
    custom_asset_t* ca = (custom_asset_t*)out;

    // Load from file or do whatever from here...

    // Set data for optional param
    ca->udata = optional_uint_param;
    gs_println("optional data: %zu, %.2f", optional_uint_param, optional_float_param);
    ca->fdata = (float)optional_float_param;
}

// Asset handles
gs_asset_t tex_hndl   = {0}; 
gs_asset_t fnt_hndl   = {0};
gs_asset_t aud_hndl   = {0};
gs_asset_t cust_hndl  = {0};
gs_asset_t cust_hndl0 = {0};

void init()
{
    gcb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();
    gsa = gs_asset_manager_new();

    // Registering custom asset importer
    gs_assets_register_importer(&gsa, custom_asset_t, &(gs_asset_importer_desc_t){
        .load_from_file = load_custom_asset_from_file
    });

    // Loading assets provided with framework
    tex_hndl = gs_assets_load_from_file(&gsa, gs_asset_texture_t, "./assets/champ.png", NULL, false);
    aud_hndl = gs_assets_load_from_file(&gsa, gs_asset_audio_t, "./assets/jump.wav");
    fnt_hndl = gs_assets_load_from_file(&gsa, gs_asset_font_t, "./assets/font.ttf", 32);

    // Create asset and get handle for custom data placed into asset manager
    custom_asset_t custom = {
        .name = "whatever", 
        .udata = 10,
        .fdata = 3.145f
    };
    cust_hndl = gs_assets_create_asset(&gsa, custom_asset_t, &custom);

    // Load custom data "from file" (can provide optional data AFTER path)
    cust_hndl0 = gs_assets_load_from_file(&gsa, custom_asset_t, "path/to/asset", 10, 2.45f);
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // Whenever user presses key, play transient sound effect
    if (gs_platform_key_pressed(GS_KEYCODE_P))
    {
        // Grab audio asset pointer from assets
        gs_asset_audio_t* ap = gs_assets_getp(&gsa, gs_asset_audio_t, aud_hndl);
        gs_audio_play_source(ap->hndl, 0.5f);
    }

    // Grab texture asset pointer from assets
    gs_asset_texture_t* tp = gs_assets_getp(&gsa, gs_asset_texture_t, tex_hndl);
    gs_asset_font_t* fp = gs_assets_getp(&gsa, gs_asset_font_t, fnt_hndl);
    custom_asset_t* cp = gs_assets_getp(&gsa, custom_asset_t, cust_hndl);
    custom_asset_t* cp0 = gs_assets_getp(&gsa, custom_asset_t, cust_hndl0);

    gsi_camera3D(&gsi);
    gsi_face_cull_enabled(&gsi, true);
    gsi_transf(&gsi, 0.f, 0.f, -5.f);
    gsi_rotatefv(&gsi, -gs_platform_elapsed_time() * 0.001f, GS_YAXIS);
    gsi_sphere(&gsi, 0.f, 0.f, 0.f, 1.5f, 20, 50, 150, 100, GS_GRAPHICS_PRIMITIVE_LINES);
    gsi_texture(&gsi, tp->hndl);
    gsi_sphere(&gsi, 0.f, 0.f, 0.f, 1.5f, 255, 255, 255, 255, GS_GRAPHICS_PRIMITIVE_TRIANGLES);

    // Default text
    gsi_defaults(&gsi);
    gsi_camera2D(&gsi);
    gsi_text(&gsi, 120.f, 100.f, "Press P to play jump sound", fp, false, 255, 255, 255, 255);

    // Print text of custom asset metric
    gs_snprintfc(cbuff, 256, "CP: %s, %zu, %.2f", cp->name, cp->udata, cp->fdata);
    gsi_text(&gsi, 175.f, 500.f, cbuff, fp, false, 255, 255, 255, 255);

    gs_snprintfc(cbuff0, 256, "CP0: %s, %zu, %.2f", cp0->name, cp0->udata, cp0->fdata);
    gsi_text(&gsi, 165.f, 550.f, cbuff0, fp, false, 255, 255, 255, 255);

    // Submit immediate draw render pass
    gsi_render_pass_submit(&gsi, &gcb, gs_color(20, 20, 20, 255));

    // Final command buffer submit
    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}