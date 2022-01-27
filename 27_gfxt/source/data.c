/*================================================================
    * Copyright: 2020 John Jackson
================================================================*/ 

typedef struct {
    gs_command_buffer_t cb;
    gs_immediate_draw_t gsi;
    gs_gfxt_pipeline_t pip;
    gs_gfxt_material_t mat;
    gs_gfxt_mesh_t mesh;
    gs_gfxt_texture_t texture; 
} app_t;

