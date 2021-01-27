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
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_UINT_DRAW_INDEX
#define GS_NK_IMPL
#define NK_IMPLEMENTATION
#include <Nuklear/nuklear.h>
#include "gs_nuklear.h"
