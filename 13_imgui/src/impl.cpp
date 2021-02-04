/*================================================================
    * Copyright: 2020 John Jackson
    * impl.cpp
================================================================*/

// Imgui defines certain stb implementations, so make sure to tell gunslinger to not define them
#define GS_NO_STB_DEFINE
#define GS_NO_STB_TRUETYPE
#define GS_NO_STB_RECT_PACK
#define GS_IMPL
#include <gs/gs.h>

#define IMGUI_IMPLEMENTATION
#include <imgui/misc/single_file/imgui_single_file.h>

#define GS_IMGUI_IMPL
#include "gs_imgui.h"

