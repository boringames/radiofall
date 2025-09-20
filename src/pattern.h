#pragma once

#include "game.h"
#include "queue.h"

typedef struct {
    i32 count;
    iVec2 coords[GRID_HEIGHT * GRID_WIDTH];
    GridColor color[GRID_HEIGHT * GRID_WIDTH];
} Pattern;

QUEUE_DECLARE(Pattern, PatternBuffer, pattbuf, 32)

bool pattern_has_coord(Pattern *p, iVec2 v);
void pattern_normalize(Pattern *p);
void pattern_rotate(Pattern *p, bool ccw);
void pattern_generate(Pattern *p);
