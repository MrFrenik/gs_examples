/*================================================================
    * Copyright: 2020 John Jackson
    * uniforms

    The purpose of this example is to demonstrate how to bind and upload 
    uniform data to the GPU using pipelines and uniform buffers.

    Included: 
        * Construct vertex/index buffers from user defined declarations
        * Construct shaders from source
        * Construct uniforms and bind data to pass to GPU
        * Construct pipelines
        * Rendering via command buffers

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

gs_command_buffer_t               cb        = {0};
gs_camera_t                       cam       = {0};
gs_handle(gs_graphics_buffer_t)   vbo       = {0};
gs_handle(gs_graphics_pipeline_t) pip       = {0};
gs_handle(gs_graphics_shader_t)   shader    = {0};
gs_handle(gs_graphics_buffer_t)   u_vp      = {0};
gs_handle(gs_graphics_buffer_t)   u_model   = {0};

const char* v_src =
    "#version 330\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout (std140) uniform u_vp {\n"
    "   mat4 projection;\n"
    "   mat4 view;\n"
    "   vec4 color;\n"
    "};\n"
    "uniform mat4 u_model;\n"
    "out vec4 f_color;\n"
    "void main() {\n"
    "   gl_Position = projection * view * u_model * vec4(a_pos, 1.0);\n"
    "   f_color = color;\n"
    "}\n";
    
const char* f_src =
    "#version 330\n"
    "layout(location = 0) out vec4 frag_color;\n"
    "in vec4 f_color;\n"
    "void main() {\n"
    "   frag_color = f_color;\n"
    "}\n";

typedef struct matrices_t
{
    gs_mat4 projection;
    gs_mat4 view;
    gs_vec4 color;
} matrices_t;

void init()
{
    // Construct new command buffer
    cb = gs_command_buffer_new(); 

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

    // Construct vertex buffer
    vbo = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t) {
            .type = GS_GRAPHICS_BUFFER_VERTEX,
            .data = v_data,
            .size = sizeof(v_data)
        }
    );

    // Shader source description
    gs_graphics_shader_source_desc_t sources[] = {
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_VERTEX, .source = v_src},
        (gs_graphics_shader_source_desc_t){.type = GS_GRAPHICS_SHADER_STAGE_FRAGMENT, .source = f_src}
    };

    // Create shader
    shader = gs_graphics_shader_create (
        &(gs_graphics_shader_desc_t) {
            .sources = sources, 
            .size = sizeof(sources),
            .name = "color_shader"
        }
    );

    /*
        // Don't define a type...
        gs_handle(gs_graphics_parameter_t) u_model = gs_graphics_parameter_create(
            &(gs_graphics_parameter_desc_t) {
                .name = "u_model"
            }
        );

        // Don't separate the two, maybe? Just place them internally in a logical manner? I dunno...

        gs_graphics_uniform_layout_desc_t uniform_layout[] = {
            GS_GRAPHICS_UNIFORM_MAT4,
            GS_GRAPHICS_UNIFORM_MAT4,
            GS_GRAPHICS_UNIFORM_VEC4
        };

            // Buffer description with all the possible parameters

        gs_graphics_buffer_desc_t desc = {
            .vertex_buffer = {
                .data = v_data,
                .size = sizeof(v_data)
            },
            .index_buffer = {
                .data = i_data,
                .size = sizeof(i_data)
            },
            .uniform_constant = {
                .stage =  GS_GRAPHICS_SHADER_STAGE_VERTEX,
                .name = "u_color",
                .layout = &(gs_graphics_uniform_type){GS_GRAPHICS_UNIFORM_VEC3}
                .layout_size = sizeof(gs_graphics_uniform_type)
            },
            .uniform_buffer = {
                .stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
                .name = "u_vp",
                .data = NULL,
                .size = sizeof(vparams_t)
                // ...Buffer range stuff as well
            },
            .sampler_buffer = {
                .stage = GS_GRAPHICS_SHADER_STAGE_FRAGMENT,
                .type = GS_GRAPHICS_SAMPLER_2D,
                .name = "u_tex"
            }
        };

        gs_graphics_buffer_update_desc_t desc = {
            .uniform_buffer = {
                .update_type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA,
                .buffer = u_vp,
                .data = data,
                .size = sizeof(data),
                .offset = 0x00
            },
            .vertex_buffer = {
                .update_type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA,
                .buffer = u_vp,
                .data = data,
                .size = sizeof(data),
                .offset = 0x00
            },
            .index_buffer = {
                .update_type = GS_GRAPHICS_BUFFER_UPDATE_SUBDATA,
                .buffer = u_vp,
                .data = data,
                .size = sizeof(data),
                .offset = 0x00
            }
        };

        // Creating a uniform buffer
        gs_handle(gs_graphics_buffer_t) u_vp = gs_graphics_buffer_create(
            &(gs_graphic_buffer_desc_t) {
                .uniform_buffer = {
                    .stage = GS_GRAPHICS_SHADER_STAGE_VERTEX,
                    .name = "u_vp",
                    .data = NULL,
                    .size = sizeof(vparams_t)
                }
            }
        );

        gs_handle(gs_graphics_buffer_t) u_vp = gs_graphics_buffer_create(
            &(gs_graphics_buffer_desc_t) {
                .type = GS_GRAPHICS_BUFFER_UNIFORM,
                .data = NULL,
                .size = sizeof(matrices_t),
                .name = "u_vp"
            }
        );

        // Updating the buffer (sub-buffer update)
        gs_graphics_buffer_request_update(
            &cb, 
            &(gs_graphics_buffer_update_desc_t){
                .uniform_buffer = {
                    .buffer = u_vp,
                    .update_type = GS_BUFFER_UPDATE_SUBDATA,
                    .data = data,
                    .size = sizeof(data)
                }
            }
        );

        gs_graphics_bind_data_t ubuffers[] = {
            (gs_graphics_bind_data_t){.hndl = u_vp, .binding = 0}
        };

        // Binds
        gs_graphics_bind_desc_t binds = 
        {
            .uniform_buffers = {.desc = ubuffers, .size = sizeof(ubuffers)},
            .uniform_constants = {.desc = uconstants, .size = sizeof(uconstants)}
        };

        gs_graphics_bind_desc_t binds = {
            .uniform_buffers = {.desc = &(gs_graphics_bind_data_desc_t){.buffer = u_vp, .binding = 0}},
            .uniform_constants = {.desc = &(gs_graphics_bind_data_desc_t){.parameter = u_model, .data = &model}},
            .vertex_buffers = {.desc = &(gs_graphics_bind_data_desc_t){.buffer = vbo}},
            .index_buffers = {.desc = &(gs_graphics_bind_data_desc_t){.buffer = ibo}}
        };

        gs_graphics_set_bindings(&cb, binds);
    */

    // Creates matrices uniform buffer
    u_vp = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t){
            .type = GS_GRAPHICS_BUFFER_UNIFORM,
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_BLOCK, .size = sizeof(matrices_t)},
            .size = sizeof(gs_graphics_uniform_desc_t),
            .name = "u_vp"
        } 
    );

    // Maybe this instead?
    // How do I get the size of the data type in order to upload the data?
    // What about an array of uniform data? For non uniform buffers?
    // Maybe a gpu param block? So it can store ALL of the data to be bound?... Hmm, but how would that work?
    /*
        // Constants must have type and size (in vulkan, push constants are blocks)
        // This is tricky, dude. Cause a uniform "struct" will have multiple uniforms...so will have to take that uniform information and then map it directly.

        // Constants
        // For vulkan, does it try to wrap all of these constants into a single buffer?
        // You can only have a single push constant per shader stage in vulkan. So that might make it to where I can 
        // combine all of these into a single "push block" to be used.
        // So when you define the constants like this for the params for this stage in this gpu param block, 
        // it'll combine them into a single push constant unit for vulkan, but they'll be individual uniforms for opengl.

        // What about binding data? That's tricky in this instance. You'd have to know how to parse the data for the constants for a particular 
        // region, right?
        // In vulkan, push constants are not named, so that wouldn't be an issue, you just have to specify the layout of the data...maybe
        // I don't know. This sucks.

        gs_handle(gs_graphics_constant_t) u_model = {0};

        u_model = gs_graphics_constant_create(&(gs_graphics_constant_desc_t){.name = "u_model"});

        // Should you be able to create a single shader constant?
        gs_handle(gs_graphics_shader_constant_t) u_model = gs_graphics_shader_constant_create(
            &(gs_graphics_shader_constant_t) {
                .name = "u_model"
            }
        );

        gs_handle(gs_graphics_buffer_t) u_ubo = gs_graphics_buffer_create(
            &(gs_graphics_buffer_desc_t) {
                .uniform = {
                    .stage = ,
                    .name = ""
                }
            }
        );

        gs_graphics_bind_buffer_desc_t v_constants[] = {
            (gs_graphics_bind_buffer_desc_t){.buffer = u_model, .data = &model}
        }

        gs_graphics_bind_desc_t binds = 
        {
            .shader = {
                .vertex = {
                    .constants = {.decl = v_constants}
                }
            } 
        };

        // Holds everything for a given shader stage needed?... jeeeeeeezus
        typedef struct gsgl_gpu_params_t
        {
            // Stages
            union {

                struct {

                    struct {
                        gs_dyn_array(gsgl_uniform_t) uniforms;
                    } vertex;

                    struct {
                        gs_dyn_array(gsgl_uniform_t) uniforms;
                    } fragment;

                } raster;

                struct {
                    gs_dyn_array(gsgl_uniform_t) uniforms;
                } compute;
            }
        } gpu_params_t;

        gs_graphics_bind_gpu_params_desc_t params_desc = {
            .buffer = gpu_params,
            .vertex = {.data = &model},
            .fragment = {.data = &}
        };

        gs_graphics_bind_gpu_params(&cb, &params_desc);

        gs_graphics_bind_buffer_desc_t params[] = {

            .shader_params = {

                .vertex = {
                    .buffers = gpu_params,
                },

                .fragment = {
                    .sampler = {}
                },
            }
        };

        uniform buffer <-> constant buffer
        uniform <-> parameter

        // You can only have a single push constant for vulkan per shader stage, but that's not the case for opengl, so whatevs

        gs_graphics_bind_desc_t binds = {
            .vertex_buffers = {.decl = &(gs_graphics_bind_buffer_desc_t){.buffer = vbo}}
            .index_buffers = ... 
            .gpu_params = {.decl = } 
        };

        // If this is an array, then in opengl it will create multiple uniforms

        // Shouldn't have to pass up type then, should be able to get that from the shader
        gs_graphics_gpu_param_constant_desc_t v_constants[] = {
            (gs_graphics_gpu_param_constant_desc_t){.name = "u_vp"},
            (gs_graphics_gpu_param_constant_desc_t){.name = "u_model"}
        };

        gs_graphics_gpu_param_constant_desc_t f_constants[] = {
            (gs_graphics_gpu_param_constant_desc_t){.name = "light.position"},
            (gs_graphics_gpu_param_constant_desc_t){.name = "light.color"}
        };

        gs_graphics_gpu_param_sampler_desc_t samplers[] = {
            (gs_graphics_gpu_param_sampler_desc_t){.type = GS_GRAPHICS_SAMPLER_2D, .name = "u_tex"}   
        };

        gs_graphics_gpu_param_image_desc_t images[] = {
        };

        gs_handle(gs_graphics_gpu_params_t) gpu_params = {0};

        // GPU params for raster shader
        // I'm okay with this API so far
        gpu_params = gs_graphics_gpu_params_create(
            &(gs_graphics_gpu_param_desc_t) {
                .raster = {
                    .vertex = {
                        .constants = {.desc = v_constants, .size = sizeof(v_constants)}
                    },
                    .fragment = {
                        .constants = {.desc = f_constants, .size = sizeof(f_constants)}
                        .samplers = {.desc = samplers, .size = sizeof(samplers)},
                    }
                }
            }
        );

        // Can you load up multiple gpu params?
        // Having to put this stuff into a 

        // Constant data to bind for vertex shader
        gs_mat4 v_constants[] = 
        {
            gs_mat4(...),  // u_vp
            gs_mat4(...)   // u_model 
        };

        gs_mat4 model = ... // So how do you push this data up?
        gs_mat4 vp = ...

        // Constant data for frag shader (light data)
        float f_constants[] = 
        {
            0.f, 0.f, 0.f,      // Position
            1.f, 1.f, 1.f, 1.f  // Color
        };

        gs_graphics_bind_buffer_desc_t gpu_params_data = 
        {
        };

        gs_graphics_bind_desc_t binds[] = {
            .shader = {{.decl = {.buffer = gpu_params, .data = gpu_params_data}}
                .buffer = gpu_params,   // Meh..
                .raster = {
                    .vertex = {.data = v_constants, .size = sizeof(v_constants)},    // Would have to know how to parse this data to line it up with multiple uniforms, and they'd have to be in order...
                    .fragment = {.data = f_constants, .size = sizeof(f_constants)}
                },
            },
            .vertex_buffers = {.decl = (...){.buffer = vbo}},
            .index_buffers = {.decl = (...){.buffer = ibo}}
        }

        // Later on...

        // Have to be able to push data to params, yes? Don't want to store any constant data, so any constant data needs to be pushed during this operation.

        gs_graphics_bind_gpu_params(&cb, gpu_params);
    */

    /*
    u_model = gs_graphics_gpu_param_create(
        &(gs_graphics_gpu_param_desc_t)
        {
            .shader_stage = stage,
            .size = sizeof(gs_mat4),        // Size of data to be passed up
            .name = "u_model"
            .constant =                     // Maybe these are the parameter qualifiers? Meh...
            .uniform_buffer = 
            .sampler = 
            .image = 
        }
    );
    */

    /*

    */

    u_model = gs_graphics_buffer_create(
        &(gs_graphics_buffer_desc_t){
            .type = GS_GRAPHICS_BUFFER_UNIFORM,
            .data = &(gs_graphics_uniform_desc_t){.type = GS_GRAPHICS_UNIFORM_MAT4},
            .size = sizeof(gs_graphics_uniform_desc_t),
            .name = "u_model"
        } 
    );

    gs_graphics_vertex_attribute_desc_t vattrs[] = {
        (gs_graphics_vertex_attribute_desc_t){.format = GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3}  // Position
    };

    pip = gs_graphics_pipeline_create (
        &(gs_graphics_pipeline_desc_t) {
            .raster = {
                .shader = shader
            },
            .blend = {
                .func = GS_GRAPHICS_BLEND_EQUATION_ADD,
                .src = GS_GRAPHICS_BLEND_MODE_SRC_ALPHA,
                .dst = GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA              
            },
            .depth = {
                .func = GS_GRAPHICS_DEPTH_FUNC_LESS
            },
            .layout = {
                .attrs = vattrs,
                .size = sizeof(vattrs)
            }
        }
    );

    cam = gs_camera_perspective();
    cam.transform.position = gs_v3(0.f, 0.f, 3.f);
}

/*
    Should push constants be their own construct? This would envelop uniforms/params/push constants etc.
    Buffer update

    gs_graphic_begin_render_pass(&cb, &pass_desc);

        gs_graphics_clear(&cb, &clear_desc);

        gs_graphics_set_viewport(&cb, view);

        gs_grpahics_set_view_scissor(&cb, scissor);

        gs_graphics_bind_pipeline(&cb, pip);

        gs_graphics_bind_bindings(&cb, &bind_desc);

        gs_graphics_draw(&cb, &draw_desc);

    gs_graphics_end_render_pass(&cb);
*/

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gs_vec2 ws = gs_platform_window_sizev(gs_platform_main_window());

    // Render pass action for clearing the screen
    gs_graphics_render_pass_action_t action = (gs_graphics_render_pass_action_t){.color = {0.1f, 0.1f, 0.1f, 1.f}};

     // fill the Model matrix array
    gs_mat4 model = gs_mat4_translate(-0.75f, 0.75f, 0.0f);

    matrices_t vp = {
        .projection = gs_camera_get_projection(&cam, (int32_t)ws.x, (int32_t)ws.y),
        .view = gs_camera_get_view(&cam),
        .color = gs_v4(1.f, 1.f, 0.f, 1.f)
    };

    gs_graphics_bind_buffer_desc_t ubuffers[] = {
        (gs_graphics_bind_buffer_desc_t){.buffer = u_vp, .data = &vp, .binding = 0},
        (gs_graphics_bind_buffer_desc_t){.buffer = u_model, .data = &model}
    };

    /*
        // Could do something like this?
        gs_graphics_bind_data_desc_t constants[] = {
            (gs_graphics_bind_data_desc_t){.hndl = u_model, .data = &model}
        };
    */

    // Binding descriptor for vertex buffer
    gs_graphics_bind_desc_t binds = {
        .vertex_buffers = {.decl = &(gs_graphics_bind_buffer_desc_t){.buffer = vbo}},
        .uniform_buffers = {.decl = ubuffers, .size = sizeof(ubuffers)}
        // .constants = {.desc = constants, .size = sizeof(constants)} 
    };

    /* Render */
    gs_graphics_begin_render_pass(&cb, (gs_handle(gs_graphics_render_pass_t)){0}, &action, sizeof(action));
        gs_graphics_bind_pipeline(&cb, pip);
        gs_graphics_bind_bindings(&cb, &binds);
        gs_graphics_draw(&cb, &(gs_graphics_draw_desc_t){.start = 0, .count = 36});
    gs_graphics_end_render_pass(&cb);

    // Submit command buffer (syncs to GPU, MUST be done on main thread where you have your GPU context created)
    gs_graphics_submit_command_buffer(&cb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   






