// data.c

#define ROW_COL_CT  10
    
// Vertex data for quad
float v_data[] = {
    // Positions  UVs
    -0.5f, -0.5f,  0.0f, 0.0f,  // Top Left
     0.5f, -0.5f,  1.0f, 0.0f,  // Top Right 
    -0.5f,  0.5f,  0.0f, 1.0f,  // Bottom Left
     0.5f,  0.5f,  1.0f, 1.0f   // Bottom Right
};

// Index data for quad
uint32_t i_data[] = {
    0, 3, 2,    // First Triangle
    0, 1, 3     // Second Triangle
};

// Shaders
#ifdef GS_PLATFORM_WEB
    #define GS_VERSION_STR "#version 300 es\n"
#else
    #define GS_VERSION_STR "#version 330 core\n"
#endif

// Shaders
const char* v_src =
GS_VERSION_STR
"layout(location = 0) in vec2 a_pos;\n"
"layout(location = 1) in vec2 a_uv;\n"
"precision mediump float;\n"
"out vec2 uv;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(a_pos, 0.0, 1.0);\n"
"   uv = a_uv;\n"
"}";

const char* f_src =
GS_VERSION_STR
"precision mediump float;\n"
"uniform sampler2D u_tex;"
"in vec2 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   frag_color = texture(u_tex, uv);\n"
"}";
