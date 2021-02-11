#ifndef __GS_PLATFORM_IMPL_EMSCRIPTEN_H__
#define __GS_PLATFORM_IMPL_EMSCRIPTEN_H__

#ifdef GS_PLATFORM_IMPL_EMSCRIPTEN

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#define GS_PLATFORM_WINDOW_MAX_TITLE_LENGTH     128

// From sokol's emscripten impl
EM_JS(void, gs_js_pointer_init, (const char* c_str_target), {
    // lookup and store canvas object by name
    var target_str = UTF8ToString(c_str_target);
    Module.gs_emsc_target = document.getElementById(target_str);
    if (!Module.gs_emsc_target) {
        console.log("gs_platform_emsc_impl.h: invalid target:" + target_str);
    }
    if (!Module.gs_emsc_target.requestPointerLock) {
        console.log("gs_platform_emsc_impl.h: target doesn't support requestPointerLock:" + target_str);
    }
});

typedef struct gs_emscripten_t {
    char canvas_selector[GS_PLATFORM_WINDOW_MAX_TITLE_LENGTH]; 
    bool32 ask_leave_site;
    const char* canvas_name; 
    bool32 canvas_resize;
    bool32 preserve_drawing_buffer;
} gs_emscripten_t;

 // Platform Init / Update / Shutdown
gs_result gs_platform_init(gs_platform_i* platform)
{
    platform->user_data = gs_malloc_init(gs_emscripten_t);
    gs_emscripten_t* ems = (gs_emscripten_t*)platform->user_data; 

    // Init state
    ems->canvas_name = "canvas";
    ems->canvas_selector[0] = '#';
    strncpy(&ems->canvas_selector[1], ems->canvas_name, gs_strlen(ems->canvas_name) - 1);

    // Other state that I'm not entirely certain about...


}

gs_result gs_platform_shutdown(gs_platform_i* platform)
{
}

// Platform Util
void gs_platform_sleep(float ms)
{
}

// Platform Input
gs_result gs_platform_process_input(gs_platform_input_t* input)
{
}

uint32_t gs_platform_key_to_codepoint(gs_platform_keycode code)
{
}

void gs_platform_mouse_set_position(uint32_t handle, float x, float y)
{
}

void* gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height)
{
}

void gs_platform_window_swap_buffer(uint32_t handle)
{
}

gs_vec2 gs_platform_window_sizev(uint32_t handle)
{
}

void gs_platform_window_size(uint32_t handle, uint32_t* width, uint32_t* height)
{
}

uint32_t gs_platform_window_width(uint32_t handle)
{
}

uint32_t gs_platform_window_height(uint32_t handle)
{
}

void gs_platform_set_window_size(uint32_t handle, uint32_t width, uint32_t height)
{
}

void gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v)
{
}

void gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor)
{
}

void gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
}

void gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
}

void gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
}

void* gs_platform_raw_window_handle(uint32_t handle)
{
}

gs_vec2 gs_platform_framebuffer_sizev(uint32_t handle)
{
}

void gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
}

uint32_t gs_platform_framebuffer_width(uint32_t handle)
{
}

uint32_t gs_platform_framebuffer_height(uint32_t handle)
{
}

// Define main frame function for engine to step
EM_BOOL gs_engine_frame()
{
    // Remove these...
    static uint32_t curr_ticks = 0; 
    static uint32_t prev_ticks = 0;

    // Cache platform pointer
    gs_platform_i* platform = gs_engine_subsystem(platform);

    // Cache times at start of frame
    platform->time.current  = gs_platform_elapsed_time();
    platform->time.update   = platform->time.current - platform->time.previous;
    platform->time.previous = platform->time.current;

    // Update platform and process input
    if (gs_platform_update(platform) != GS_RESULT_IN_PROGRESS) {
        gs_engine_instance()->shutdown();
        return false;
    }

    // Process application context
    gs_engine_instance()->ctx.app.update();
    if (!gs_engine_instance()->ctx.app.is_running) {
        gs_engine_instance()->shutdown();
        return false;
    }

    // NOTE(John): This won't work forever. Must change eventually.
    // Swap all platform window buffers? Sure...
    for 
    (
        gs_slot_array_iter it = 0;
        gs_slot_array_iter_valid(platform->windows, it);
        gs_slot_array_iter_advance(platform->windows, it)
    )
    {
        gs_platform_window_swap_buffer(it);
    }

    // Frame locking (not sure if this should be done here, but it is what it is)
    platform->time.current  = gs_platform_elapsed_time();
    platform->time.render   = platform->time.current - platform->time.previous;
    platform->time.previous = platform->time.current;
    platform->time.frame    = platform->time.update + platform->time.render;            // Total frame time
    platform->time.delta    = platform->time.frame / 1000.f;

    float target = (1000.f / platform->time.max_fps);

    if (platform->time.frame < target)
    {
        gs_platform_sleep((float)(target - platform->time.frame));
        
        platform->time.current = gs_platform_elapsed_time();
        double wait_time = platform->time.current - platform->time.previous;
        platform->time.previous = platform->time.current;
        platform->time.frame += wait_time;
        platform->time.delta = platform->time.frame / 1000.f;
    }
}

/* Main entry point */
int32_t main(int32_t argc, char** argv)
{
    gs_engine_t* inst = gs_engine_create(gs_main(argv, argc));
    emscripten_set_main_loop(gs_engine_frame, 0, true);
    return 0;
}

#endif

#endif __GS_PLATFORM_IMPL_EMSCRIPTEN_H__