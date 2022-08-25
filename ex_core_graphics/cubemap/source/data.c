// data.c

// Vertex data for skybox
float v_data[] = {
    -1.0f,  1.0f,  1.0f, 
    -1.0f, -1.0f,  1.0f, 
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, 
    -1.0f,  1.0f, -1.0f, 
    -1.0f, -1.0f, -1.0f, 
     1.0f, -1.0f, -1.0f, 
     1.0f,  1.0f, -1.0f 
};

uint16_t i_data[] = {
   0, 1, 2, 3,
   3, 2, 6, 7,
   7, 6, 5, 4,
   4, 5, 1, 0,
   0, 3, 7, 4, // T
   1, 2, 6, 5  // B
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
"layout(location = 0) in vec3 a_pos;\n"
"precision mediump float;\n"
"uniform mat4 u_proj;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_model;\n"
"out vec3 uv;\n"
"void main()\n"
"{\n"
"   uv = a_pos;\n"
"   vec4 pos = u_proj * u_view * u_model * vec4(a_pos, 1.0);\n"
"   gl_Position = pos.xyww;\n"
"}";

const char* f_src =
GS_VERSION_STR
"precision mediump float;\n"
"uniform samplerCube u_tex;\n"
"in vec3 uv;\n"
"out vec4 frag_color;\n"
"void main()\n"
"{\n"
"   vec4 tex_color = texture(u_tex, uv);\n"
"   frag_color = vec4(tex_color.rgb, 1.0);\n"
"}";



