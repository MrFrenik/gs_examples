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

    Press `esc` to exit the application.
================================================================*/

#define GS_IMPL
#include <gs/gs.h>

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

// Keys for slot map
const char* smkeys[ITER_CT] = 
{
    "John", 
    "Dick", 
    "Harry", 
    "Donald",
    "Wayne"
};

void init()
{
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
    }

    print_console();
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // To print array
    if (gs_platform_key_pressed(GS_KEYCODE_ONE)) 
    {
        gs_printf("gs_dyn_array: [");
        for (uint32_t i = 0; i < gs_dyn_array_size(arr); ++i)
        {
            gs_printf("%zu, ", arr[i]);
        }
        gs_println("]");
    }

    // Iterate hash table
    if (gs_platform_key_pressed(GS_KEYCODE_TWO))
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
    if (gs_platform_key_pressed(GS_KEYCODE_THREE))
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
    if (gs_platform_key_pressed(GS_KEYCODE_FOUR))
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
    if (gs_platform_key_pressed(GS_KEYCODE_FIVE))
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
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update,
        .shutdown = cleanup
    };
}






