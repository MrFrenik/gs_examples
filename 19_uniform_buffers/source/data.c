// data.c

#ifdef GS_PLATFORM_WEB
    #define GS_GL_VERSION_STR "#version 300 es\n"
#else
    #define GS_GL_VERSION_STR "#version 430 core\n"
#endif

const char* v_src =
    GS_GL_VERSION_STR
    "layout(location = 0) in vec3 a_pos;\n"
    "layout (std140) uniform u_vp {\n"
    "   mat4 projection;\n"
    "   mat4 view;\n"
    "};\n"
    "layout (std430, binding = 2) buffer u_voxels_buffer {\n"
    "   int data;\n"
    "};\n"
    "uniform mat4 u_model;\n"
    "void main() {\n"
    "   uint v = data[0];\n"
    "   gl_Position = projection * view * u_model * vec4(a_pos, 1.0);\n"
    "}\n";

const char* f_red_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

const char* f_blue_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(0.0, 0.0, 1.0, 1.0);\n"
    "}\n";

const char* f_green_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

const char* f_yellow_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(0.0, 1.0, 1.0, 1.0);\n"
    "}\n";

// Cube positions
float v_data[] = {
    // positions
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,

     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,

    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,

    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
};
