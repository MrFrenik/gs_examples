// data.c

float v_data[] = {
    // positions     // colors
    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
    -0.05f, -0.05f,  0.0f, 0.0f, 1.0f,

    -0.05f,  0.05f,  1.0f, 0.0f, 0.0f,
     0.05f, -0.05f,  0.0f, 1.0f, 0.0f,
     0.05f,  0.05f,  0.0f, 1.0f, 1.0f
};

// Shaders
#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

const char* v_src =
GS_VERSION_STR
"precision mediump float;\n"
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec3 a_color;\n"
"layout(location = 2) in vec2 a_offset;\n"
"out vec3 f_color;\n"
"void main()\n"
"{\n"
"   vec2 pos = a_pos * (float(gl_InstanceID) / 100.0);\n"
"   gl_Position = vec4(pos + a_offset, 0.0, 1.0);\n"
"   f_color = a_color;\n"
"}";

const char* f_src =
GS_VERSION_STR
"precision mediump float;\n"
"in vec3 f_color;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(f_color, 1.0);\n"
"}";

gs_vec2 g_translations[100] = {0};

