#pragma once

#include <raylib.h>
#include "core/types.h"

#define GRID_WIDTH 8
#define GRID_HEIGHT 12
#define GRID_CELL_SIDE 16

typedef enum GridColor {
    COLOR_EMPTY,
    COLOR_BLUE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_COUNT,
} GridColor;

static const iVec2 DIRS[4] = { IVEC2(1, 0), IVEC2(-1, 0), IVEC2(0, 1), IVEC2(0, -1) };
