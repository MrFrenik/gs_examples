/*================================================================
    * Copyright: 2020 John Jackson
    * meta_util

    The purpose of this example is to demonstrate how to use the meta 
    util for creating register meta information about custom user types.

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_IMMEDIATE_DRAW_IMPL
#include <gs/util/gs_idraw.h>

#define GS_META_IMPL
#include <gs/util/gs_meta.h>

#define TMPSTRSZ  1024

typedef struct custom_struct_t 
{
    gs_vec2 v2val;
    uint64_t u64val;
} custom_struct_t;

// Declare custom property type info
#define GS_META_PROPERTY_TYPE_CUSTOM        (GS_META_PROPERTY_TYPE_COUNT + 1)
#define GS_META_PROPERTY_TYPE_INFO_CUSTOM   _gs_meta_property_type_decl(custom_struct_t, GS_META_PROPERTY_TYPE_CUSTOM)

// Type to reflect
typedef struct thing_t
{
    float fval;
    uint32_t uval;
    int32_t sval;
    gs_vec3 v3val;
    gs_quat qval;
    custom_struct_t csval;
} thing_t;

// Globals
gs_command_buffer_t gcb = {0}; 
gs_immediate_draw_t gsi = {0};
gs_meta_registry_t  gmr = {0};
thing_t             thing = {0};

void print_object(void* obj, const gs_meta_class_t* cls, gs_vec2* pos)
{
    char buf[TMPSTRSZ] = {0};

    #define DO_TEXT(TXT)\
        do {\
            gsi_text(&gsi, pos->x, pos->y, buf, NULL, false, 255, 255, 255, 255);\
            pos->y += 20.f;\
            memset(buf, 0, TMPSTRSZ);\
        } while (0)

    gs_snprintf(buf, TMPSTRSZ, "cls: %s", cls->name);
    DO_TEXT(TMPSTRSZ);

    // Iterate through property count of class
    for (uint32_t i = 0; i < cls->property_count; ++i)
    {
        // Get property at i
        gs_meta_property_t* prop = &cls->properties[i];

        // Depending on property, we'll handle the debug text differently
        switch (prop->type.id)
        {
            case GS_META_PROPERTY_TYPE_U8: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint8_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S8: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int8_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U16: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint16_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S16: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int16_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint32_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int32_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_U64:
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %zu", prop->type.name, prop->name, gs_meta_getv(obj, uint64_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_S64: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %d", prop->type.name, prop->name, gs_meta_getv(obj, int64_t, prop));
            } break;

            case GS_META_PROPERTY_TYPE_F32: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %.2f", prop->type.name, prop->name, gs_meta_getv(obj, float, prop));
            } break;

            case GS_META_PROPERTY_TYPE_F64: 
            {
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): %.2f", prop->type.name, prop->name, gs_meta_getv(obj, double, prop));
            } break;

            case GS_META_PROPERTY_TYPE_VEC2: 
            {
                gs_vec2* v = gs_meta_getvp(obj, gs_vec2, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f>", prop->type.name, prop->name, v->x, v->y);
            } break;

            case GS_META_PROPERTY_TYPE_VEC3: 
            {
                gs_vec3* v = gs_meta_getvp(obj, gs_vec3, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z);
            } break;

            case GS_META_PROPERTY_TYPE_VEC4: 
            {
                gs_vec4* v = gs_meta_getvp(obj, gs_vec4, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z, v->w);
            } break;

            case GS_META_PROPERTY_TYPE_QUAT: 
            {
                gs_quat* v = gs_meta_getvp(obj, gs_quat, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): <%.2f, %.2f, %.2f>", prop->type.name, prop->name, v->x, v->y, v->z, v->w);
            } break;

            case GS_META_PROPERTY_TYPE_MAT4:
            {
                gs_mat4* m = gs_meta_getvp(obj, gs_mat4, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>\
                    \n\t<%.2f, %.2f, %.2f, %.2f>", 
                    prop->type.name, prop->name, 
                    m->elements[0], m->elements[1], m->elements[2], m->elements[3],
                    m->elements[4], m->elements[5], m->elements[6], m->elements[7],
                    m->elements[8], m->elements[9], m->elements[10], m->elements[11],
                    m->elements[12], m->elements[13], m->elements[14], m->elements[15]
                );
            } break;

            case GS_META_PROPERTY_TYPE_VQS: 
            {
                gs_vqs* v = gs_meta_getvp(obj, gs_vqs, prop);
                gs_snprintf(buf, TMPSTRSZ, "%s (%s): pos: <%.2f, %.2f, %.2f>, rot: <%.2f, %.2f, %.2f, %.2f>, scale: <%.2f, %.2f, %.2f>", 
                    prop->type.name, prop->name, 
                    v->position.x, v->position.y, v->position.z,
                    v->rotation.x, v->rotation.y, v->rotation.z, v->rotation.w,
                    v->scale.x, v->scale.y, v->scale.z
                );
            } break;

            case GS_META_PROPERTY_TYPE_CUSTOM:
            {
                // Get other class for this type, then pass in to this function recursively to display
                gs_meta_class_t* clz = gs_meta_get_class(&gmr, custom_struct_t);
                if (clz)
                {
                    // Get value at this property to pass into print function
                    custom_struct_t* cs = gs_meta_getvp(obj, custom_struct_t, prop);
                    // Do temp print for this property name and type
                    gs_snprintf(buf, TMPSTRSZ, "%s (%s):", prop->type.name, prop->name);
                    DO_TEXT(TMPSTRSZ);
                    // Tab x value
                    pos->x += 10.f;
                    // Print object
                    print_object(cs, clz, pos);
                    // Return x value to original
                    pos->x -= 10.f;
                }

            } break;
        }

        DO_TEXT(TMPSTRSZ);
    }
}

void app_init()
{
    gcb = gs_command_buffer_new();
    gsi = gs_immediate_draw_new();
    gmr = gs_meta_registry_new();

    // Construct instance of thing struct
    thing = (thing_t) {
        .fval = 3.145f,
        .uval = 64,
        .sval = -20,
        .v3val = gs_v3(1, 2, 3),
        .qval = gs_quat_default(),
        .csval = (custom_struct_t){
            .v2val = gs_v2(2, 4),
            .u64val = 123
        }
    };

    // Register meta class information for thing (returns id, if needed)
    uint64_t thing_cls_id = gs_meta_register_class(&gmr, thing_t, (&(gs_meta_class_decl_t){
        .properties = (gs_meta_property_t[]) {
            gs_meta_property(thing_t, fval, GS_META_PROPERTY_TYPE_INFO_F32),        // Default provided types
            gs_meta_property(thing_t, uval, GS_META_PROPERTY_TYPE_INFO_U32),
            gs_meta_property(thing_t, sval, GS_META_PROPERTY_TYPE_INFO_S32),
            gs_meta_property(thing_t, v3val, GS_META_PROPERTY_TYPE_INFO_VEC3),
            gs_meta_property(thing_t, qval, GS_META_PROPERTY_TYPE_INFO_QUAT),
            gs_meta_property(thing_t, csval, GS_META_PROPERTY_TYPE_INFO_CUSTOM)     // Custom property type info declared above
        },
        .size = 6 * sizeof(gs_meta_property_t)
    }));

    // Register meta class information for custom struct (returns id, if needed)
    uint64_t cs_cls_id = gs_meta_register_class(&gmr, custom_struct_t, (&(gs_meta_class_decl_t){
        .properties = (gs_meta_property_t[]) {
            gs_meta_property(custom_struct_t, v2val, GS_META_PROPERTY_TYPE_INFO_VEC2),
            gs_meta_property(custom_struct_t, u64val, GS_META_PROPERTY_TYPE_INFO_U64)
        },
        .size = 2 * sizeof(gs_meta_property_t)
    }));
}

void app_update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gsi_camera2D(&gsi);

    // Present property information (debug text)
    gs_meta_class_t* cls = gs_meta_get_class(&gmr, thing_t);

    // Do print
    gs_vec2 pos = gs_v2(100.f, 100.f);
    print_object(&thing, cls, &pos);

    // Submit immediate draw render pass
    gsi_render_pass_submit(&gsi, &gcb, gs_color(20, 20, 20, 255));

    // Final command buffer submit
    gs_graphics_submit_command_buffer(&gcb);
}

void app_shutdown()
{
    // Free meta registry data
    gs_meta_registry_free(&gmr);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = app_init,
        .update = app_update,
        .shutdown = app_shutdown
    };
}




