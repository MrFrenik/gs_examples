#ifndef ENGINE_APP_H
#define ENGINE_APP_H

#include <string>

#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <gs/util/gs_asset.h>

class App {
public:
    App();
    virtual ~App();

    virtual void Init();
    virtual void Update();
    virtual void Shutdown();

    void Run();

    // Placing assets and retrieving
    gs_asset_t GetAsset(const std::string& name);
    void PlaceAsset(const std::string& name, gs_asset_t hndl);

    // Resource loading
    void LoadTexture(const std::string& name, const std::string& path, gs_graphics_texture_desc_t* desc = nullptr);
    void LoadFont(const std::string& name, const std::string& path, uint32_t font_point = 16);
    void LoadAudio(const std::string& name, const std::string& path);

public:
    gs_command_buffer_t m_gcb;  // Used for submitting rendering commands to graphics backend
    gs_immediate_draw_t m_gsi;  // Immediate draw utility context
    gs_asset_manager_t m_gsa;   // Asset manager for storing textures
    gs_app_desc_t m_app;        // Main app description
    gs_hash_table(u64, gs_asset_t) m_assetTable; // Name -> Asset Handle Lookup
};

#endif // !ENGINE_APP_H 