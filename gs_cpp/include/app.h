#ifndef ENGINE_APP_H
#define ENGINE_APP_H

#include <unordered_map>
#include <vector>
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

    void LoadTexture(std::string name, std::string path, gs_graphics_texture_desc_t* desc = nullptr);

    gs_immediate_draw_t* GetImmediateDraw(); 
    gs_command_buffer_t* GetCommandBuffer();

public:
    gs_command_buffer_t m_gcb; // Used for submitting rendering commands to graphics backend
    gs_immediate_draw_t m_gsi; // Immediate draw utility context
    gs_asset_manager_t m_gsa;  // Asset manager for storing textures
    gs_app_desc_t m_app;
    std::unordered_map<std::string, gs_asset_t> m_assetTable; // Name -> Asset Handle lookup
};

#endif // !ENGINE_APP_H 