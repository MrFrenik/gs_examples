// Data for program

#define ROW_COL_CT  10
#define CAM_SPEED   10

// set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cube_vdata[] = {
        // positions          // texture Coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    float plane_vdata[] = {
        // positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

         5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
        -5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
         5.0f, -0.5f, -5.0f,  2.0f, 2.0f
    };

#ifdef GS_PLATFORM_WEB
    #define GS_GL_VERSION_STR "#version 300 es\n"
#else
    #define GS_GL_VERSION_STR "#version 330 core\n"
#endif

const char* v_src =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec2 a_texcoord;\n"
    "struct vp_t {\n"
    "   mat4 proj;\n"
    "   mat4 view;\n"
    "};\n"
    "uniform vp_t u_vp;\n"
    "uniform mat4 u_model;\n"
    "out vec2 tex_coord;\n"
    "void main() {\n"
    "   tex_coord = a_texcoord;\n"
    "   gl_Position = u_vp.proj * u_vp.view * u_model * vec4(a_pos, 1.0);\n"
    "}\n";

const char* f_texsrc =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "in vec2 tex_coord;\n"
    "uniform sampler2D u_tex;\n"
    "void main() {\n"
    "   frag_color = texture(u_tex, tex_coord);\n"
    "}\n";

const char* f_colsrc =
    GS_GL_VERSION_STR
    "precision mediump float;\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "void main() {\n"
    "   frag_color = vec4(0.04, 0.28, 0.26, 1.0);\n"
    "}\n";

// Helper macro for filling pixel data with checkered texture based on two given colors (C0, C1)
#define generate_texture(PIXELS, C0, C1)\
    do {\
        for (uint32_t r = 0; r < ROW_COL_CT; ++r) {\
            for (uint32_t c = 0; c < ROW_COL_CT; ++c) {\
                const bool re = (r % 2) == 0;\
                const bool ce = (c % 2) == 0;\
                uint32_t idx = r * ROW_COL_CT + c;\
                PIXELS[idx] = (re && ce) ? (C0) : (re) ? (C1) : (ce) ? (C1) : (C0);\
            }\
        }\
    } while (0)

