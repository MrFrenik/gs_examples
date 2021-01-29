#ifndef __GS_IMGUI_IMPL_H__
#define __GS_IMGUI_IMPL_H__

// Main context for necessary imgui information
typedef struct gs_imgui_t
{
    uint32_t win_hndl;  
    double time;
    bool32 mouse_just_pressed[ImGuiMouseButton_COUNT]; 
    bool32 mouse_cursors[ImGuiMouseCursor_COUNT];
    gs_handle(gs_graphics_pipeline_t) pip;
    gs_handle(gs_graphics_buffer_t) vbo;
    gs_handle(gs_graphics_buffer_t) ibo;
    gs_handle(gs_graphics_shader_t) shader;
    gs_handle(gs_graphics_texture_t) font_tex; 
    gs_handle(gs_graphics_buffer_t) u_tex;
    gs_handle(gs_graphics_buffer_t) u_proj;
} gs_imgui_t;

IMGUI_IMPL_API bool     gs_imgui_init(gs_imgui_t* gs, uint32_t hndl, bool install_callbacks);
IMGUI_IMPL_API void     gs_imgui_new_frame(gs_imgui_t* gs);

/*===============================
//      Implementation
================================*/

#ifdef GS_IMGUI_IMPL

IMGUI_IMPL_API bool     
gs_imgui_init(gs_imgui_t* gs, uint32_t hndl, bool install_callbacks)
{
    gs->win_hndl = hndl;
    gs->time = 0.0;

    // Setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_gs";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab]         = gs_platform_key_to_codepoint(GS_KEYCODE_TAB);
    io.KeyMap[ImGuiKey_LeftArrow]   = gs_platform_key_to_codepoint(GS_KEYCODE_LEFT);
    io.KeyMap[ImGuiKey_RightArrow]  = gs_platform_key_to_codepoint(GS_KEYCODE_RIGHT);
    io.KeyMap[ImGuiKey_UpArrow]     = gs_platform_key_to_codepoint(GS_KEYCODE_UP);
    io.KeyMap[ImGuiKey_DownArrow]   = gs_platform_key_to_codepoint(GS_KEYCODE_DOWN);
    io.KeyMap[ImGuiKey_PageUp]      = gs_platform_key_to_codepoint(GS_KEYCODE_PGUP);
    io.KeyMap[ImGuiKey_PageDown]    = gs_platform_key_to_codepoint(GS_KEYCODE_PGDOWN);
    io.KeyMap[ImGuiKey_Home]        = gs_platform_key_to_codepoint(GS_KEYCODE_HOME);
    io.KeyMap[ImGuiKey_End]         = gs_platform_key_to_codepoint(GS_KEYCODE_END);
    io.KeyMap[ImGuiKey_Insert]      = gs_platform_key_to_codepoint(GS_KEYCODE_INSERT);
    io.KeyMap[ImGuiKey_Delete]      = gs_platform_key_to_codepoint(GS_KEYCODE_DELETE);
    io.KeyMap[ImGuiKey_Backspace]   = gs_platform_key_to_codepoint(GS_KEYCODE_BSPACE);
    io.KeyMap[ImGuiKey_Space]       = gs_platform_key_to_codepoint(GS_KEYCODE_SPACE);
    io.KeyMap[ImGuiKey_Enter]       = gs_platform_key_to_codepoint(GS_KEYCODE_ENTER);
    io.KeyMap[ImGuiKey_Escape]      = gs_platform_key_to_codepoint(GS_KEYCODE_ESC);
    io.KeyMap[ImGuiKey_KeyPadEnter] = gs_platform_key_to_codepoint(GS_KEYCODE_NPENTER);
    io.KeyMap[ImGuiKey_A]           = gs_platform_key_to_codepoint(GS_KEYCODE_A);
    io.KeyMap[ImGuiKey_C]           = gs_platform_key_to_codepoint(GS_KEYCODE_C);
    io.KeyMap[ImGuiKey_V]           = gs_platform_key_to_codepoint(GS_KEYCODE_V);
    io.KeyMap[ImGuiKey_X]           = gs_platform_key_to_codepoint(GS_KEYCODE_X);
    io.KeyMap[ImGuiKey_Y]           = gs_platform_key_to_codepoint(GS_KEYCODE_Y);
    io.KeyMap[ImGuiKey_Z]           = gs_platform_key_to_codepoint(GS_KEYCODE_Z);

    // Rendering
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    const char* glsl_version = "#version 130";

//     io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
//     io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
//     io.ClipboardUserData = g_Window;
// #if defined(_WIN32)
//     io.ImeWindowHandle = (void*)glfwGetWin32Window(g_Window);
// #endif

    // Create mouse cursors
    // (By design, on X11 cursors are user configurable and some cursors may be missing. When a cursor doesn't exist,
    // GLFW will emit an error which will often be printed by the app, so we temporarily disable error reporting.
    // Missing cursors will return NULL and our _UpdateMouseCursor() function will use the Arrow cursor instead.)
//     GLFWerrorfun prev_error_callback = glfwSetErrorCallback(NULL);
//     g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
// #if GLFW_HAS_NEW_CURSORS
//     g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
// #else
//     g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
//     g_MouseCursors[ImGuiMouseCursor_NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
// #endif
//     glfwSetErrorCallback(prev_error_callback);

    // Chain GLFW callbacks: our callbacks will call the user's previously installed callbacks, if any.
    // g_PrevUserCallbackMousebutton = NULL;
    // g_PrevUserCallbackScroll = NULL;
    // g_PrevUserCallbackKey = NULL;
    // g_PrevUserCallbackChar = NULL;
    // if (install_callbacks)
    // {
    //     g_InstalledCallbacks = true;
    //     g_PrevUserCallbackMousebutton = glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
    //     g_PrevUserCallbackScroll = glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
    //     g_PrevUserCallbackKey = glfwSetKeyCallback(window, ImGui_ImplGlfw_KeyCallback);
    //     g_PrevUserCallbackChar = glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    // }

    // g_ClientApi = client_api;

    return true;
}

void gs_imgui_update_mouse_and_keys(gs_imgui_t* ctx)
{
    ImGuiIO& io = ImGui::GetIO();

    // Need to add an input character...check to see if it's an alpha/numeric string

    // Have to poll events from platform layer to do this
    gs_platform_event_t evt = {};
    while(gs_platform_poll_event(&evt))
    {
        switch (evt.type)
        {
            case GS_PLATFORM_EVENT_KEY:
            {
                switch (evt.key.action)
                {
                    case GS_PLATFORM_KEY_DOWN:
                    case GS_PLATFORM_KEY_PRESSED:
                    {
                        // Not sure if this is correct at all.
                        uint32_t cp = evt.key.codepoint;
                        if (cp <= IM_UNICODE_CODEPOINT_MAX)
                        {
                            io.AddInputCharacter(cp); 
                        }

                        io.KeysDown[evt.key.codepoint] = true;

                    } break;

                    case GS_PLATFORM_KEY_RELEASED:
                    {
                        io.KeysDown[evt.key.codepoint] = false;
                    } break;

                    default: break;
                }
            } break;

            default: break;
        }
    }

    // Modifiers
    io.KeyCtrl   = gs_platform_key_down(GS_KEYCODE_LCTRL) || gs_platform_key_down(GS_KEYCODE_RCTRL);
    io.KeyShift  = gs_platform_key_down(GS_KEYCODE_LSHIFT) || gs_platform_key_down(GS_KEYCODE_RSHIFT);
    io.KeyAlt    = gs_platform_key_down(GS_KEYCODE_LALT) || gs_platform_key_down(GS_KEYCODE_RALT);
    io.KeySuper  = false;

// #ifdef GS_PLATFORM_WIN
//     io.KeySuper = false;
// #else
//     io.KeySuper = 
// #endif

    // Update buttons
    io.MouseDown[0] = gs_platform_mouse_down(GS_MOUSE_LBUTTON);
    io.MouseDown[1] = gs_platform_mouse_down(GS_MOUSE_MBUTTON);
    io.MouseDown[2] = gs_platform_mouse_down(GS_MOUSE_RBUTTON);

    int32_t mpx = 0, mpy = 0;
    gs_platform_mouse_position(&mpx, &mpy);
    io.MousePos = ImVec2((float)mpx, (float)mpy);

    // Update buttons
//     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
//     {
//         // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
//         io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(g_Window, i) != 0;
//         g_MouseJustPressed[i] = false;
//     }

//     // Update mouse position
//     const ImVec2 mouse_pos_backup = io.MousePos;
//     io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
// #ifdef __EMSCRIPTEN__
//     const bool focused = true; // Emscripten
// #else
//     const bool focused = glfwGetWindowAttrib(g_Window, GLFW_FOCUSED) != 0;
// #endif
//     if (focused)
//     {
//         if (io.WantSetMousePos)
//         {
//             glfwSetCursorPos(g_Window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
//         }
//         else
//         {
//             double mouse_x, mouse_y;
//             glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
//             io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
//         }
//     }
}

IMGUI_IMPL_API void     
gs_imgui_new_frame(gs_imgui_t* gs)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    uint32_t w, h;
    uint32_t display_w, display_h;

    // Get platform window size and framebuffer size from window handle
    gs_platform_window_size(gs->win_hndl, &w, &h);
    gs_platform_framebuffer_size(gs->win_hndl, &display_w, &display_h);

    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step
    double current_time = (double)gs_platform_elapsed_time();
    io.DeltaTime = gs->time > 0.0 ? (float)(current_time - gs->time) : (float)(1.0f / 60.0f);
    gs->time = current_time;

    gs_imgui_update_mouse_and_keys(gs);

    // ImGui_ImplGlfw_UpdateMousePosAndButtons();
    // ImGui_ImplGlfw_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    // ImGui_ImplGlfw_UpdateGamepads();
}

#endif // GS_IMGUI_IMPL

#endif // __GS_IMGUI_IMPL_H__