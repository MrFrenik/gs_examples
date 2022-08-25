// Data for program 

#ifdef GS_PLATFORM_WEB
    #define GS_GL_VERSION_STR "#version 300 es\n"
#else
    #define GS_GL_VERSION_STR "#version 330 core\n"
#endif

const char* v_src =
    GS_GL_VERSION_STR
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec2 a_texcoord;\n"
    "uniform mat4 u_proj;\n"
    "uniform mat4 u_view;\n"
    "uniform mat4 u_model;\n"
    "out vec2 uv;\n"
    "void main() {\n"
    "   gl_Position = u_proj * u_view * u_model * vec4(a_pos, 1.0);\n"
    "   uv = a_texcoord;\n"
    "}\n";

const char* f_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "in vec2 uv;\n"
    "uniform sampler2D u_tex;\n"
    "void main() {\n"
    "   frag_color = vec4(1.0, 0.0, 0.0, 1.0); //texture(u_tex, uv);\n"
    "}\n";
