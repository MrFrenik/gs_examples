/*================================================================
    * Copyright: 2020 John Jackson
    * meta_util

    The purpose of this example is to demonstrate how to use the meta 
    util for creating register meta information about custom and supplied 
    gunslinger types.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_META_IMPL
#include <gs/util/gs_meta.h>

// Type to reflect
typedef struct thing_t
{
    float fval;
    uint32_t uval;
    int32_t sval;
} thing_t;

// Globals
gs_command_buffer_t gcb = {0}; 
gs_immediate_draw_t gsi = {0};
gs_meta_registry_t  gmr = {0};
thing_t             thing = {0};

void init()
{
    gcb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();
    gmr = gs_meta_registry_new();

    thing = (thing_t){
        .fval = 3.145f,
        .uval = 64,
        .sval = -20
    };

    // Construct delcaration for meta class registry
    gs_meta_property_t properties[] = {
        (gs_meta_property_t){.name = gs_to_str(fval), .offset = gs_offset(thing_t, fval), .type = GS_META_PROPERTY_TYPE_F32},
        (gs_meta_property_t){.name = gs_to_str(uval), .offset = gs_offset(thing_t, uval), .type = GS_META_PROPERTY_TYPE_U32},
        (gs_meta_property_t){.name = gs_to_str(sval), .offset = gs_offset(thing_t, sval), .type = GS_META_PROPERTY_TYPE_S32}
    };

    gs_meta_class_decl_t decl = {
        .properties = properties,
        .properties_size = sizeof(properties)
    };

    // Register meta class information
    gs_meta_register_class(&gmr, thing_t, &decl);
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gsi_camera2D(&gsi);

    // Present property information (debug text)
    gs_vec2 pos = gs_v2(100.f, 100.f);
    gs_meta_class_t* cls = gs_meta_get_class(&gmr, thing_t);
    char tmpbuf[256] = {0};

    for (uint32_t i = 0; i < cls->property_count; ++i)
    {
        gs_meta_property_t* prop = &cls->properties[i];

        switch (prop->type)
        {
            case GS_META_PROPERTY_TYPE_F32: {
                gs_snprintf(tmpbuf, 256, "%s: %.2f", prop->name, gs_meta_getv(&thing, f32, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U32: {
                gs_snprintf(tmpbuf, 256, "%s: %zu", prop->name, gs_meta_getv(&thing, u32, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S32: {
                gs_snprintf(tmpbuf, 256, "%s: %d", prop->name, gs_meta_getv(&thing, s32, prop));
            } break;
        }

        gsi_text(&gsi, pos.x, pos.y, tmpbuf, NULL, false, 255, 255, 255, 255);
        pos.y += 20.f;
        memset(tmpbuf, 0, 256);
    }

    // Submit immediate draw render pass
    gsi_render_pass_submit(&gsi, &gcb, gs_color(20, 20, 20, 255));

    // Final command buffer submit
    gs_graphics_submit_command_buffer(&gcb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}




