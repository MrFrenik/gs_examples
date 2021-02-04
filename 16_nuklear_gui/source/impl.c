/*================================================================
    * Copyright: 2020 John Jackson
    * impl.c

    // All necessary implementations will be here
================================================================*/

// Nuklear defines certain stb implementations, so make sure to tell gunslinger to not define them
#define GS_NO_STB_TRUETYPE
#define GS_NO_STB_RECT_PACK

// Define gunslinger implementation
#define GS_IMPL
#include <gs/gs.h>

// Define necessary nuklear implementation
// #define GS_NK_MOUSE_GRABBING
#define NK_IMPLEMENTATION
#define GS_NK_IMPL
#include "gs_nk_incl.h"
