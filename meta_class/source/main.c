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

typedef struct another_thing_t
{
    uint32_t fval;
    uint32_t uval;
    float sval;
} another_thing_t;

// Globals
gs_command_buffer_t gcb = {0}; 
gs_immediate_draw_t gsi = {0};
gs_meta_registry_t  gmr = {0};
thing_t             thing = {0};

size_t gs_obj_hash_impl(gs_byte_buffer_t* bb, void* obj, size_t len)
{
    gs_byte_buffer_seek_to_beg(bb); 
    gs_byte_buffer_write_bulk(bb, obj, len);
    gs_byte_buffer_seek_to_beg(bb);
    memset(obj, 0, len);
    size_t id = gs_hash_bytes(obj, len, GS_HASH_TABLE_HASH_SEED);
    gs_byte_buffer_read_bulk(bb, &obj, len);
    return id;
}

#define gs_obj_hash(BB, OBJ)\
    gs_obj_hash_impl(BB, OBJ, sizeof(*OBJ))

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
        gs_meta_property_decl(fval, thing_t, GS_META_PROPERTY_TYPE_F32),
        gs_meta_property_decl(uval, thing_t, GS_META_PROPERTY_TYPE_U32),
        gs_meta_property_decl(sval, thing_t, GS_META_PROPERTY_TYPE_S32)
    };

    gs_meta_class_decl_t decl = {
        .properties = properties,
        .properties_size = sizeof(properties)
    };

    // Register meta class information
    gs_meta_register_class(&gmr, thing_t, &decl);

    thing_t t = {0};
    another_thing_t at = {0};

    gs_byte_buffer_t bb = gs_byte_buffer_new();

    // gs_byte_buffer_write_bulk(&bb, &thing, sizeof(thing));
    // gs_byte_buffer_seek_to_beg(&bb);

    size_t hash0 = gs_hash_bytes(&t, sizeof(t), GS_HASH_TABLE_HASH_SEED);
    size_t hash3 = gs_obj_hash(&bb, &thing);
    size_t hash4 = gs_obj_hash(&bb, &at);

    gs_println("h0: %lu", hash0);
    gs_println("h3: %lu", hash3);
    gs_println("h4: %lu", hash4);
}

// size_t gs_hash_bytes(void *p, size_t len, size_t seed)
// uint64_t get_key(void* bytes)
// {
// } 

// #define hash_type_bytes(DATA)\


void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    gsi_camera2D(&gsi);

    // Present property information (debug text)
    gs_vec2 pos = gs_v2(100.f, 100.f);
    gs_meta_class_t* cls = gs_meta_get_class(&gmr, thing_t);
    char buf[32] = {0};

    for (uint32_t i = 0; i < cls->property_count; ++i)
    {
        gs_meta_property_t* prop = &cls->properties[i];

        switch (prop->type)
        {
            case GS_META_PROPERTY_TYPE_U8: gs_snprintf(buf, 32, "%s: %zu", prop->name, gs_meta_getv(&thing, uint8_t, prop));   break;
            case GS_META_PROPERTY_TYPE_S8: gs_snprintf(buf, 32, "%s: %d", prop->name, gs_meta_getv(&thing, int8_t, prop));     break;
            case GS_META_PROPERTY_TYPE_U16: gs_snprintf(buf, 32, "%s: %zu", prop->name, gs_meta_getv(&thing, uint16_t, prop)); break;
            case GS_META_PROPERTY_TYPE_S16: gs_snprintf(buf, 32, "%s: %d", prop->name, gs_meta_getv(&thing, int16_t, prop));   break;
            case GS_META_PROPERTY_TYPE_U32: gs_snprintf(buf, 32, "%s: %zu", prop->name, gs_meta_getv(&thing, uint32_t, prop)); break;
            case GS_META_PROPERTY_TYPE_S32: gs_snprintf(buf, 32, "%s: %d", prop->name, gs_meta_getv(&thing, int32_t, prop));   break;
            case GS_META_PROPERTY_TYPE_U64: gs_snprintf(buf, 32, "%s: %lu", prop->name, gs_meta_getv(&thing, uint64_t, prop)); break;
            case GS_META_PROPERTY_TYPE_S64: gs_snprintf(buf, 32, "%s: %d", prop->name, gs_meta_getv(&thing, int64_t, prop));   break;
            case GS_META_PROPERTY_TYPE_F32: gs_snprintf(buf, 32, "%s: %.2f", prop->name, gs_meta_getv(&thing, float, prop));   break;
            case GS_META_PROPERTY_TYPE_F64: gs_snprintf(buf, 32, "%s: %2f", prop->name, gs_meta_getv(&thing, double, prop));   break;
        }

        gsi_text(&gsi, pos.x, pos.y, buf, NULL, false, 255, 255, 255, 255);
        pos.y += 20.f;
        memset(buf, 0, 32);
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




