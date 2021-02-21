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
            - gs_asset_mesh_t
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

gs_command_buffer_t                     gcb = {0}; 
gs_immediate_draw_t                     gsi = {0};
gs_asset_manager_t                      gsa = {0};

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
    ca->fdata = (float)optional_float_param;
}

// Asset handles
gs_asset_t tex_hndl   = {0};
gs_asset_t fnt_hndl   = {0};
gs_asset_t aud_hndl   = {0};
gs_asset_t msh_hndl   = {0};
gs_asset_t cust_hndl  = {0};
gs_asset_t cust_hndl0 = {0};
gs_asset_t dtex_hndl  = {0};

void init()
{
    gcb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();
    gsa = gs_asset_manager_new();

    // Registering custom asset importer
    gs_assets_register_importer(&gsa, custom_asset_t, &(gs_asset_importer_desc_t){
        .load_from_file = load_custom_asset_from_file
    });

    // Mesh layout for loading mesh to use with immediate rendering 
    // (if not provided, will construct layout based on mesh data when loaded from file)
    gs_asset_mesh_layout_t mesh_layout[] = {
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_POSITION},  // Float3
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_TEXCOORD},  // Float2
        (gs_asset_mesh_layout_t){.type = GS_ASSET_MESH_ATTRIBUTE_TYPE_COLOR}      // Byte4
    };

    gs_asset_mesh_decl_t mesh_decl = {
        .layout = mesh_layout,
        .layout_size = sizeof(mesh_layout)
    };

    gs_asset_texture_t tex0 = {0};
    gs_asset_texture_t tex1 = {0};
    gs_asset_audio_t   aud  = {0};
    gs_asset_mesh_t    msh  = {0};
    gs_asset_font_t    fnt  = {0};

    // Load asset data
    gs_asset_texture_load_from_file("./assets/champ.png", &tex0, NULL, false, false);
    gs_asset_audio_load_from_file("./assets/jump.wav", &aud);
    gs_asset_font_load_from_file("./assets/font.ttf", &fnt, 32);
    gs_asset_mesh_load_from_file("./assets/duck/Duck.gltf", &msh, &mesh_decl, NULL, 0);
    gs_asset_texture_load_from_file("./assets/duck/DuckCM.png", &tex1, NULL, false, false);

    // Create and place assets
    tex_hndl = gs_assets_create_asset(&gsa, gs_asset_texture_t, &tex0);
    dtex_hndl = gs_assets_create_asset(&gsa, gs_asset_texture_t, &tex1);
    aud_hndl = gs_assets_create_asset(&gsa, gs_asset_audio_t, &aud);
    fnt_hndl = gs_assets_create_asset(&gsa, gs_asset_font_t, &fnt);
    msh_hndl = gs_assets_create_asset(&gsa, gs_asset_mesh_t, &msh);

    // Create asset and get handle for custom data placed into asset manager
    // Might have to just do this instead. Load from file just isn't going to work.
    custom_asset_t custom = (custom_asset_t){
        .name = "whatever", 
        .udata = 10,
        .fdata = 3.145f
    };
    cust_hndl = gs_assets_create_asset(&gsa, custom_asset_t, &custom);

    custom = (custom_asset_t){
        .name = "whatever", 
        .udata = 2,
        .fdata = 2.45
    };

    // // Load custom data "from file" (can provide optional data AFTER path)
    cust_hndl0 = gs_assets_create_asset(&gsa, custom_asset_t, &custom);
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    const gs_vec2 fb = gs_platform_framebuffer_sizev(gs_platform_main_window());
    const gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Whenever user presses key, play transient sound effect
    if (gs_platform_key_pressed(GS_KEYCODE_SPACE)) {
        // Grab audio asset pointer from assets
        gs_asset_audio_t* ap = gs_assets_getp(&gsa, gs_asset_audio_t, aud_hndl);
        gs_println("playing sound");
        gs_audio_play_source(ap->hndl, 0.5f);
    }

    // Grab texture asset pointer from assets
    gs_asset_texture_t* tp = gs_assets_getp(&gsa, gs_asset_texture_t, tex_hndl);
    custom_asset_t* cp = gs_assets_getp(&gsa, custom_asset_t, cust_hndl);
    custom_asset_t* cp0 = gs_assets_getp(&gsa, custom_asset_t, cust_hndl0);
    gs_asset_font_t* fp = gs_assets_getp(&gsa, gs_asset_font_t, fnt_hndl);
    gs_asset_mesh_t* mp = gs_assets_getp(&gsa, gs_asset_mesh_t, msh_hndl);
    gs_asset_texture_t* dtp = gs_assets_getp(&gsa, gs_asset_texture_t, dtex_hndl);

    gsi_camera3D(&gsi);
    gsi_face_cull_enabled(&gsi, true);
    gsi_transf(&gsi, -2.f, 0.f, -5.f);
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

    gs_graphics_clear_desc_t clear = {.actions = &(gs_graphics_clear_action_t){.color = 0.1f, 0.1f, 0.1f, 255}};

    // Bind render pass for backbuffer
    gs_graphics_begin_render_pass(&gcb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
        gs_graphics_set_viewport(&gcb, 0, 0, (int32_t)fb.x, (int32_t)fb.y);
        gs_graphics_clear(&gcb, &clear);
        gsi_draw(&gsi, &gcb); // Binds pipelines and submits to graphics command buffer for rendering

        // Get pipeline for mesh from immediate draw backend
        gsi_pipeline_state_attr_t state = {
            .depth_enabled = 1,
            .stencil_enabled = 0,
            .blend_enabled = 0,
            .face_cull_enabled = 1,
            .prim_type = (uint16_t)GS_GRAPHICS_PRIMITIVE_TRIANGLES
        };
        gs_handle(gs_graphics_pipeline_t) pip = gsi_get_pipeline(&gsi, state);

        gs_graphics_bind_pipeline(&gcb, pip); // Bind pipeline

        // MVP Matrix
        gs_mat4 mvp = gs_mat4_perspective(60.f, ws.x / ws.y, 0.1f, 1000.f);
        mvp = gs_mat4_mul_list(
            6, 
            mvp,
            gs_mat4_translate(8.f, -1.f, -30.f),
            gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0001f, GS_YAXIS), 
            gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0005f, GS_XAXIS), 
            gs_mat4_rotatev(gs_platform_elapsed_time() * 0.0002f, GS_ZAXIS), 
            gs_mat4_scale(0.05f, 0.05f, 0.05f)
        );

        // For each primitive in mesh
        for (uint32_t i = 0; i < gs_dyn_array_size(mp->primitives); ++i)
        {
            gs_asset_mesh_primitive_t* prim = &mp->primitives[i];

            // Bindings for all buffers: vertex, index, uniform, sampler
            gs_graphics_bind_desc_t binds = {
                .vertex_buffers = {.desc = &(gs_graphics_bind_vertex_buffer_desc_t){.buffer = prim->vbo}},
                .index_buffers = {.desc = &(gs_graphics_bind_index_buffer_desc_t){.buffer = prim->ibo}},
                .uniforms = {
                    .desc = (gs_graphics_bind_uniform_desc_t[]){
                        {.uniform = gsi.uniform, .data = &mvp},
                        {.uniform = gsi.sampler, .data = &dtp->hndl, .binding = 0}
                    },
                    .size = 2 * sizeof(gs_graphics_bind_uniform_desc_t)
                }
            };

            gs_graphics_apply_bindings(&gcb, &binds);
            gs_graphics_draw(&gcb, &(gs_graphics_draw_desc_t){.start = 0, .count = prim->count});
        }

    gs_graphics_end_render_pass(&gcb);

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