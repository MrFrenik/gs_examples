/*================================================================
    * Copyright: 2020 John Jackson
    * containers

    The purpose of this example is to demonstrate how to use 
    provided generic containers.

    Included: 
        * gs_dyn_array(T): dynamic array of type 'T' 
        * gs_hash_table(K, V): hash table of key 'K' : value 'V'
        * gs_slot_array(T): double indirection array of type 'T'
        * gs_slot_map(K, V): indirection array of type 'V' with custom key type 'K'
        * gs_byte_buffer_t: uint8_t data buffer, capable of storing any type of mixed data

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

#define GS_ASSET_IMPL
#include <gs/util/gs_asset.h>

#define ITER_CT   5

// Helper macro for printing console commands
#define print_console()\
    do {\
        gs_println("(1): Print dynamic array");\
        gs_println("(2): Print hash table");\
        gs_println("(3): Print hash table with custom key");\
        gs_println("(4): Print slot array");\
        gs_println("(5): Print slot map");\
    } while(0)

typedef struct custom_key_t 
{
    uint32_t uval;
    float fval;
} custom_key_t;

gs_dyn_array(uint32_t) arr = NULL;                // Dynamic Array
gs_hash_table(float, uint32_t) ht = NULL;         // Hash table
gs_hash_table(custom_key_t, uint32_t) htc = NULL; // Hash table with custom key struct
gs_slot_array(double) sa = NULL;                  // Slot array
gs_slot_map(uint64_t, uint32_t) sm = NULL;        // Slot map (hashed str64 bit key)
gs_byte_buffer_t bb = {0};                        // Byte buffer

// Keys for slot map
const char* smkeys[ITER_CT] = 
{
    "John", 
    "Dick", 
    "Harry", 
    "Donald",
    "Wayne"
};

// Custom asset definition
typedef struct custom_asset_t
{
    const char* name;
    uint32_t udata;
    float fdata;
} custom_asset_t;

gs_asset_manager_t am = {0};

void init()
{
    am = gs_asset_manager_new();

    // Registering custom asset importer
    gs_assets_register_importer(&am, custom_asset_t, &(gs_asset_importer_desc_t){0});

    gs_assert(gs_hash_table_key_exists(am.importers, gs_hash_str64(gs_to_str(gs_asset_texture_t)))); 

    // Construct byte buffer
    bb = gs_byte_buffer_new();

    // Write total amount to be written into byte buffer
    gs_byte_buffer_write(&bb, uint32_t, ITER_CT);

    // Insertion
    for (uint32_t i = 0; i < ITER_CT; ++i) 
    {
        // Dynamic Array
        gs_dyn_array_push(arr, i);

        // Hash table
        gs_hash_table_insert(ht, (float)i, i);

        // Hash table with custom key
        custom_key_t k = {.uval = i, .fval = (float)i * 2.f};
        gs_hash_table_insert(htc, k, i * 2);

        // Slot array
        gs_slot_array_insert(sa, (double)i * 3.f);

        // Slot map
        gs_slot_map_insert(sm, gs_hash_str64(smkeys[i]), i);

        // Byte buffer write
        gs_byte_buffer_write(&bb, uint32_t, i);
    }

    // Rewind byte buffer to beginning for reading
    gs_byte_buffer_seek_to_beg(&bb);

    print_console();
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // To print array
    if (gs_platform_key_pressed(GS_KEYCODE_1)) 
    {
        gs_printf("gs_dyn_array: [");
        for (uint32_t i = 0; i < gs_dyn_array_size(arr); ++i)
        {
            gs_printf("%zu, ", arr[i]);
        }
        gs_println("]");
    }

    // Iterate hash table
    if (gs_platform_key_pressed(GS_KEYCODE_2))
    {
        gs_println("gs_hash_table: [");
        for (
            gs_hash_table_iter it = gs_hash_table_iter_new(ht);
            gs_hash_table_iter_valid(ht, it);
            gs_hash_table_iter_advance(ht, it)
        )
        {
            float k = gs_hash_table_iter_getk(ht, it);
            uint32_t v = gs_hash_table_iter_get(ht, it);
            gs_println("  {k: %.2f, v: %zu},", k, v);
        }
        gs_println("]");
    }

    // Iterate custom key hash table
    if (gs_platform_key_pressed(GS_KEYCODE_3))
    {
        gs_println("gs_hash_table: [");
        for (
            gs_hash_table_iter it = gs_hash_table_iter_new(htc);
            gs_hash_table_iter_valid(htc, it);
            gs_hash_table_iter_advance(htc, it)
        )
        {
            custom_key_t* kp = gs_hash_table_iter_getkp(htc, it);
            uint32_t v = gs_hash_table_iter_get(htc, it);
            gs_println("  {k: {%zu, %.2f}, v: %zu},", kp->uval, kp->fval, v);
        }
        gs_println("]");
    }

    // Iterate slot array
    if (gs_platform_key_pressed(GS_KEYCODE_4))
    {
        gs_println("gs_slot_array: [");
        for (
            gs_slot_array_iter it = gs_slot_array_iter_new(sa);
            gs_slot_array_iter_valid(sa, it);
            gs_slot_array_iter_advance(sa, it)
        )
        {
            double v = gs_slot_array_iter_get(sa, it);    
            gs_println("  id: %zu, v: %.2f", it, v);
        }
        gs_println("]");
    }

    // Iterate slot map
    if (gs_platform_key_pressed(GS_KEYCODE_5))
    {
        // Find data by hashed string key
        gs_println("gs_slot_map (manual): [");
        for (uint32_t i = 0; i < ITER_CT; ++i)
        {
            uint32_t v = gs_slot_map_get(sm, gs_hash_str64(smkeys[i]));
            gs_println("k: %s, h: %lu, v: %zu", smkeys[i], gs_hash_str64(smkeys[i]), v);
        }
        gs_println("]");

        gs_println("gs_slot_map (iterator): [");
        for (
            gs_slot_map_iter it = gs_slot_map_iter_new(sm);
            gs_slot_map_iter_valid(sm, it);
            gs_slot_map_iter_advance(sm, it)
        )
        {
            uint64_t k = gs_slot_map_iter_getk(sm, it);
            uint32_t v = gs_slot_map_iter_get(sm, it);
            gs_println("k: %lu, v: %zu", k, v);  
        }
        gs_println("]");
    }

    if (gs_platform_key_pressed(GS_KEYCODE_6))
    {
        // Find data by hashed string key
        gs_println("gs_byte_buffer_t: [");

        // Read count from byte buffer (construct new variable with readc function)
        gs_byte_buffer_readc(&bb, uint32_t, ct);

        for (uint32_t i = 0; i < ct; ++i)
        {
            // Read back uint32_t from buffer
            gs_byte_buffer_readc(&bb, uint32_t, v);
            gs_println("v: %zu", v);
        }
        gs_println("]");

        // Seek back to beginning to read again
        gs_byte_buffer_seek_to_beg(&bb);
    }
}

void cleanup()
{
    // Clear all data
    gs_dyn_array_clear(arr);
    gs_hash_table_clear(ht);
    gs_hash_table_clear(htc);
    gs_slot_array_clear(sa);
    gs_slot_map_clear(sm);

    // Free all data
    gs_dyn_array_free(arr);
    gs_hash_table_free(ht);
    gs_hash_table_free(htc);
    gs_slot_array_free(sa);
    gs_slot_map_free(sm);
    gs_byte_buffer_free(&bb);
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update,
        .shutdown = cleanup
    };
}






