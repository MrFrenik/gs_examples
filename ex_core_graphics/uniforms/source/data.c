// data.c

// Vertex data for triangle
float v_data[] = {
    0.0f, 0.5f,
    -0.5f, -0.5f, 
    0.5f, -0.5f
};

#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

const char* v_src =
GS_VERSION_STR
"layout(location = 0) in vec2 a_pos;\n"
"precision mediump float;\n"
"uniform mat4 u_model;\n"
"void main()\n"
"{\n"
"   gl_Position = u_model * vec4(a_pos, 0.0, 1.0);\n"
"}";

const char* f_src =
GS_VERSION_STR
"precision mediump float;\n"
"out vec4 frag_color;\n"
"uniform vec3 u_color;\n"
"void main()\n"
"{\n"
"   frag_color = vec4(u_color, 1.0);\n"
"}";

