#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <gs/gs.h>

// Emscripten context data 
typedef struct ems_t
{
    const char* canvas_name;
    double canvas_width;
    double canvas_height;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx;
} ems_t;

#define EMS_DATA()\
    ((ems_t*)(gs_engine_subsystem(platform)->user_data))

EM_BOOL ems_size_changed_cb(int32_t type, const EmscriptenUiEvent* evt, void* user_data)
{
    gs_platform_t* platform = gs_engine_subsystem(platform);
    ems_t* ems = (ems_t*)platform->user_data;
    (void)type;
    (void)evt;
    (void)user_data;
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);
    emscripten_set_canvas_element_size(ems->canvas_name, ems->canvas_width, ems->canvas_height);
    return true;
}

EM_BOOL ems_keypress_cb(int32_t type, const EmscriptenKeyboardEvent* evt, void* user_data) 
{
    (void)type;
    (void)user_data;
    if (evt->keyCode < 512) {
        gs_println("key pressed: %zu", evt->keyCode);
    }
    return evt->keyCode < 32;
}

EM_BOOL ems_keydown_cb(int32_t type, const EmscriptenKeyboardEvent* evt, void* user_data) 
{
    (void)type;
    (void)user_data;
    if (evt->keyCode < 512) {
        gs_println("key down: %zu", evt->keyCode);
    }
    return evt->keyCode < 32;
}

EM_BOOL ems_keyup_cb(int32_t type, EmscriptenKeyboardEvent* evt, void* user_data)
{
    (void)type;   
    (void)user_data;
    if (evt->keyCode < 512) {
        gs_println("key up: %zu", evt->keyCode); 
    }
    return evt->keyCode < 32;
}

EM_BOOL ems_mousedown_cb(int32_t type, EmscriptenMouseEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_println("button down: %zu", evt->button);
    return true;
}

EM_BOOL ems_mouseup_cb(int32_t type, EmscriptenMouseEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_println("button down: %zu", evt->button);
    return true;
}

EM_BOOL ems_mousepress_cb(int32_t type, EmscriptenMouseEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_println("button down: %zu", evt->button);
    return true;
}

EM_BOOL ems_mousemove_cb(int32_t type, EmscriptenMouseEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_println("mouse move: %.2f, %.2f", (float)evt->targetX, (float)evt->targetY);
    return true;
}

EM_BOOL ems_mousewheel_cb(int32_t type, EmscriptenWheelEvent* evt, void* user_data)
{
    (void)type;
    (void)user_data;
    gs_println("mouse wheel: %.2f, %.2f", (float)evt->deltaX, (float)evt->deltaY);
    return true;
}

GS_API_DECL void       
gs_platform_init(gs_platform_t* platform)
{
    gs_println("Initializing Emscripten.");

    gs_app_desc_t* app = gs_engine_app();
    platform->user_data = gs_malloc_init(ems_t);
    ems_t* ems = (ems_t*)platform->user_data;

    // Just set this to defaults for now
    ems->canvas_name = "canvas";
    emscripten_set_canvas_element_size(ems->canvas_name, app->window_width, app->window_height);
    emscripten_get_element_css_size(ems->canvas_name, &ems->canvas_width, &ems->canvas_height);

    // Set up callbacks
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, false, ems_size_changed_cb);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, ems_keydown_cb);
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, ems_keypress_cb);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, true, ems_keyup_cb);
    emscripten_set_mousedown_callback("#canvas", NULL, true, ems_mousedown_cb);
    emscripten_set_mouseup_callback("#canvas", NULL, true, ems_mousedown_cb);
    emscripten_set_click_callback("#canvas", NULL, true, ems_mousepress_cb);
    emscripten_set_click_callback("#canvas", NULL, true, ems_mouseup_cb);

    // Set up webgl context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.antialias = false;
    attrs.depth = true;
    attrs.premultipliedAlpha = false;
    attrs.stencil = true;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    attrs.enableExtensionsByDefault = true;
    ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    if (!ems->ctx) {
        gs_println("Emscripten Init: Unable to create webgl2 context. Reverting to webgl1.");
        attrs.majorVersion = 1;
        ems->ctx = emscripten_webgl_create_context(ems->canvas_name, &attrs);
    } else {
        gs_println("Emscripten Init: Successfully created webgl2 context.");
    }
    if (emscripten_webgl_make_context_current(ems->ctx) != EMSCRIPTEN_RESULT_SUCCESS) {
        gs_println("Emscripten Init: Unable to set current webgl context.");
    }
}

GS_API_DECL void       
gs_platform_shutdown(gs_platform_t* platform)
{
    // Free memory
}

GS_API_DECL double 
gs_platform_elapsed_time()
{
    return emscripten_performance_now(); 
}

// Platform Video
GS_API_DECL void 
gs_platform_enable_vsync(int32_t enabled)
{
    // Nothing for now...
}

// Platform Util
GS_API_DECL void   
gs_platform_sleep(float ms)
{
    emscripten_sleep((uint32_t)ms);   
}

// Platform Input
GS_API_DECL void 
gs_platform_process_input(gs_platform_input_t* input)
{
}

GS_API_DECL uint32_t  
gs_platform_key_to_codepoint(gs_platform_keycode code)
{
    return 0;
}

GS_API_DECL void      
gs_platform_mouse_set_position(uint32_t handle, float x, float y)
{
}

GS_API_DECL void*    
gs_platform_create_window_internal(const char* title, uint32_t width, uint32_t height)
{
    // Nothing for now, since we just create this internally...
    return NULL;
}

GS_API_DECL void     
gs_platform_window_swap_buffer(uint32_t handle)
{
    // Nothing for emscripten...
}

GS_API_DECL gs_vec2  
gs_platform_window_sizev(uint32_t handle)
{
    ems_t* ems = EMS_DATA();
    return gs_v2((float)ems->canvas_width, (float)ems->canvas_height);
}

GS_API_DECL void     
gs_platform_window_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    ems_t* ems = EMS_DATA();
    *w = (uint32_t)ems->canvas_width;
    *h = (uint32_t)ems->canvas_height;
}

GS_API_DECL uint32_t 
gs_platform_window_width(uint32_t handle)
{
    return (uint32_t)EMS_DATA()->canvas_width;
}

GS_API_DECL uint32_t 
gs_platform_window_height(uint32_t handle)
{
    return (uint32_t)EMS_DATA()->canvas_height;
}

GS_API_DECL void     
gs_platform_set_window_size(uint32_t handle, uint32_t width, uint32_t height)
{
    ems_t* ems = EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, width, height);
    ems->canvas_width = (uint32_t)width;
    ems->canvas_height = (uint32_t)height;
}

GS_API_DECL void     
gs_platform_set_window_sizev(uint32_t handle, gs_vec2 v)
{
    ems_t* ems = EMS_DATA();
    emscripten_set_canvas_element_size(ems->canvas_name, (uint32_t)v.x, (uint32_t)v.y);
    ems->canvas_width = (uint32_t)v.x;
    ems->canvas_height = (uint32_t)v.y;
}

GS_API_DECL void     
gs_platform_set_cursor(uint32_t handle, gs_platform_cursor cursor)
{
}

GS_API_DECL void     
gs_platform_set_dropped_files_callback(uint32_t handle, gs_dropped_files_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_window_close_callback(uint32_t handle, gs_window_close_callback_t cb)
{
}

GS_API_DECL void     
gs_platform_set_character_callback(uint32_t handle, gs_character_callback_t cb)
{
}

GS_API_DECL void*    
gs_platform_raw_window_handle(uint32_t handle)
{
    return NULL;
}

GS_API_DECL gs_vec2  
gs_platform_framebuffer_sizev(uint32_t handle)
{
    ems_t* ems = EMS_DATA();
    return gs_v2((float)ems->canvas_width, (float)ems->canvas_height);
}

GS_API_DECL void     
gs_platform_framebuffer_size(uint32_t handle, uint32_t* w, uint32_t* h)
{
    ems_t* ems = EMS_DATA();
    *w = (uint32_t)ems->canvas_width;
    *h = (uint32_t)ems->canvas_height;
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_width(uint32_t handle)
{
    // Get ems width for now. Don't use handle.
    return (uint32_t)EMS_DATA()->canvas_width;
}

GS_API_DECL uint32_t 
gs_platform_framebuffer_height(uint32_t handle)
{
    return (uint32_t)EMS_DATA()->canvas_height;
}

int32_t main(int32_t argc, char** argv)
{
    gs_app_desc_t app = gs_main(argc, argv);
    gs_engine_create(app);
    emscripten_set_main_loop(gs_engine_frame, (int32_t)app.frame_rate, true);
    return 0;
}













