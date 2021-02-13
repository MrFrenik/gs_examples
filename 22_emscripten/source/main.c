#define GS_PLATFORM_IMPL_DEFAULT
#define GS_PLATFORM_IMPL_CUSTOM
#define GS_IMPL
#include <gs/gs.h>

// Avoid gs platform layer? Could try that.
// See, how would you use gunslinger with a custom platform layer though? Without having to use GS's platform layer stuff.
// Don't want to have to have the user create, compile, include custom implementations for layers with every project
// Remove GLFW, but provide it as an optional platform impl

gs_command_buffer_t cb = {0};

void app_init() 
{
    cb = gs_command_buffer_new();
}

void app_update() 
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    const float t = gs_platform_elapsed_time();
    // gs_println("t: %.2f", t);
    float r = ((sin(t * 0.001f) * 0.5f + 0.5f));
    float g = ((sin(t * 0.002f) * 0.5f + 0.5f));
    float b = ((sin(t * 0.003f) * 0.5f + 0.5f));

    // You should be able to create a graphics instance, right? Then just use that.
    /*
        // Remove "engine" from this. Should just be gs_context(), which returns the global context, which isn't a pointer.
        // By default, will initialize everything in context.
        app_desc_t {
            .systems = GS_SYSTEM_ALL    -> initialize all systems by default 
        };

        // Not sure.
        app_desc_t {
            .init_flags = GS_INIT_AUDIO | GS_INIT_PLATFORM | GS_INIT_GRAPHICS     -> just the audio and graphics systems (could be possible if the graphics system is software rendered, I suppose?)
        };

        // The context holds pointers to static systems. These static pointers can then either be allocated internally by the system, or they can be set manually by the user.
        // Just init audio
        gs_context_t* ctx = gs_setup(gs_app_desc_t*);

        gs_graphics_t gfx = {0};
        gs_graphics_init(&gfx); 
        gs_graphics_set_current(&gfx);

        gs_graphics_begin_render_pass(&cb, GS_GRPAHICS_RENDER_PASS_DEFAULT);
        gs_graphics_clear(&cb, clear);
    */

    // Try to clear screen?
    gs_graphics_clear_desc_t clear = (gs_graphics_clear_desc_t){.actions = &(gs_graphics_clear_action_t){.color = {r, g, b, 1.f}}};
    gs_graphics_begin_render_pass(&cb, GS_GRAPHICS_RENDER_PASS_DEFAULT);
    gs_graphics_clear(&cb, &clear);
    gs_graphics_end_render_pass(&cb);
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t) {
        .init = app_init,
        .update = app_update,
        .window_width = 500,
        .window_height = 400
    }; 
}

/*
    Write custom platform layer - how? Would be nice to have specific defaults then ONLY write what I need to override. But that requires either setting EVERYTHING bounded in macros (disgusting), 
    or using a vtable (also disgusting).

	// Platform impl

    int32_t main(int32_t argc, char** argv)
    {
    	// Application with various settings for the layers.
    	gs_app_desc_t app = {
			.platform = {
			},
			.audio = {
			},
			.graphics = {
			}
    	};

    	gs_app_desc_t app = {0};
    	gs_platform_t* platform = gs_platform_setup(&app);
    	gs_graphics_t* gfx = gs_graphics_setup(&app);
    	gs_audio_t* audio = gs_graphics_audio_setup(&app);

    	// Good to go, now.

    	gs_setup(&app);	 -> sets up contexts for all of those.
    }
    
*/























