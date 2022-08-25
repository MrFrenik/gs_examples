/*================================================================
    * Copyright: 2020 John Jackson
    * compute_shader

    The purpose of this example is to demonstrate how to construct a compute
    shader, dispatch it to render to a texture, then use the immediate draw 
    util to render that texture to the screen.

    Shader modified from: http://wili.cc/blog/opengl-cs.html

    Included: 
        * Construct compute shaders
        * Construct compute pipelines
        * Rendering via immediate mode utility

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

gs_command_buffer_t               cb      = {0};
gs_immediate_draw_t               gsi     = {0};
gs_handle(gs_graphics_uniform_t)  u_roll  = {0};
gs_handle(gs_graphics_texture_t)  cmptex  = {0};
gs_handle(gs_graphics_pipeline_t) cmdpip  = {0};
gs_handle(gs_graphics_shader_t)   cmpshd  = {0};
gs_handle(gs_graphics_storage_buffer_t)  u_voxels    = {0};

#define TEX_WIDTH  512
#define TEX_HEIGHT 512

#define CHUNK_BYTES (sizeof(uint32_t) * 32)

const char* comp_src =
    "#version 430 core\n"
    "uniform float u_roll;\n"
    "layout(rgba32f, binding = 0) uniform image2D destTex;\n"
    "layout (std430, binding = 1) readonly buffer u_voxels {\n"
    "   vec4 data;\n"
    "};\n"
    "layout (local_size_x = 16, local_size_y = 16) in;\n"
    "void main() {\n"
        "ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);\n"
        "float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy) - 8 ) / 8.0);\n"
        "float globalCoef = sin(float(gl_WorkGroupID.x + gl_WorkGroupID.y) * 0.1 + u_roll) * 0.5;\n" 
        "vec4 rc = vec4(1.0 - globalCoef * localCoef, globalCoef * localCoef, 0.0, 1.0);\n"
        "vec4 color = mix(rc, data, 0.5f);\n" 
        "imageStore(destTex, storePos, color);\n"
    "}";

void app_init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new(); 
    gsi = gs_immediate_draw_new(gs_platform_main_window());

    gs_graphics_info_t* info = gs_graphics_info();
    if (!info->compute.available) {
        gs_println("Warning: Compute shaders not available.");
        return;
    }

    // Compute shader
    gs_graphics_shader_source_desc_t sources[] = {
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_COMPUTE, .source = comp_src}
    };

    // Create shader
    cmpshd = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = sources, 
            .size = sizeof(sources),
            .name = "compute"
        }
    );

    // Create uniform
    u_roll = gs_graphics_uniform_create (
        &(gs_graphics_uniform_desc_t) {
            .name = "u_roll",                                                         
            .layout = &(gs_graphics_uniform_layout_desc_t){.type = GS_GRAPHICS_UNIFORM_FLOAT}
        }
    );

    // Texture for compute shader output
    cmptex = gs_graphics_texture_create (
        &(gs_graphics_texture_desc_t) {
            .width = TEX_WIDTH,
            .height = TEX_HEIGHT, 
            .wrap_s = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .wrap_t = GS_GRAPHICS_TEXTURE_WRAP_REPEAT,
            .min_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
            .mag_filter = GS_GRAPHICS_TEXTURE_FILTER_NEAREST,
            .format = GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F,
            .data = NULL
        }
    );

    cmdpip = gs_graphics_pipeline_create    (
        &(gs_graphics_pipeline_desc_t) {
            .compute = {
                .shader = cmpshd
            }
        }
    );

    gs_vec4 data = gs_v4(0.f, 1.f, 1.f, 1.f);

    u_voxels = gs_graphics_storage_buffer_create(
        &(gs_graphics_storage_buffer_desc_t){
            .data = &data,
            .size = sizeof(gs_vec4),
            .name = "u_voxels",
            .usage = GS_GRAPHICS_BUFFER_USAGE_DYNAMIC
        }
    ); 
} 

void app_update()
{
    const gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());

    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_quit();

    // Simply return if we can't do this sample.
    gs_graphics_info_t* info = gs_graphics_info();
    if (!info->compute.available) {
        return;
    } 

    // Compute pass
    {
        float roll = gs_platform_elapsed_time() * .001f;

        // Bindings for compute shader
        gs_graphics_bind_desc_t binds = {
            .uniforms = {.desc = &(gs_graphics_bind_uniform_desc_t){.uniform = u_roll, .data = &roll}},
            .storage_buffers = {&(gs_graphics_bind_storage_buffer_desc_t){.buffer = u_voxels, .binding = 1}},
            .image_buffers = {.desc = &(gs_graphics_bind_image_buffer_desc_t){.tex = cmptex, .access = GS_GRAPHICS_ACCESS_WRITE_ONLY, .binding = 0}}
        };

        // Bind compute pipeline
        gs_graphics_pipeline_bind(&cb, cmdpip);
        // Bind compute bindings
        gs_graphics_apply_bindings(&cb, &binds);
        // Dispatch compute shader
        gs_graphics_dispatch_compute(&cb, TEX_WIDTH / 16, TEX_HEIGHT / 16, 1);
    }

    // Use immediate mode rendering to display texture
    gsi_camera2D(&gsi, fbs.x, fbs.y);
    gsi_texture(&gsi, cmptex);
    gsi_rectvd(&gsi, gs_v2(0.f, 0.f), gs_v2((float)TEX_WIDTH, (float)TEX_HEIGHT), gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), GS_COLOR_WHITE, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    gsi_renderpass_submit(&gsi, &cb, fbs.x, fbs.y, gs_color(10, 10, 10, 255));

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_command_buffer_submit(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update,
        .window_width = TEX_WIDTH,
        .window_height = TEX_HEIGHT,
        .window_flags = GS_WINDOW_FLAGS_NO_RESIZE
    };
}   






