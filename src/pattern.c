#include "pattern.h"

#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>

QUEUE_DEFINE(Pattern, PatternBuffer, pattbuf, 32)

bool pattern_has_coord(Pattern *p, iVec2 v)
{
    for (i32 i = 0; i < p->count; i++) {
        if (ivec2_eq(v, p->coords[i])) {
            return true;
        }
    }
    return false;
}

void pattern_normalize(Pattern *p)
{
    // find minimum x,y of coordinates into pattern
    // then subtract those to each coords
    i32 miny = INT_MAX;
    i32 minx = INT_MAX;
    for (i32 i = 0; i < p->count; i++) {
        miny = MIN(p->coords[i].y, miny);
        minx = MIN(p->coords[i].x, minx);
    }
    for (i32 i = 0; i < p->count; i++) {
        p->coords[i].y -= miny;
        p->coords[i].x -= minx;
    }
}

void pattern_rotate(iVec2 base_pos, Pattern *p)
{
    for (i32 i = 0; i < p->count; i++) {
        Vector2 v = Vector2Rotate(as_vec2(p->coords[i]), M_PI/2);
        p->coords[i] = IVEC2(round(v.x), round(v.y));
    }
}

void pattern_generate(Pattern *p)
{
    i32 count = GetRandomValue(3, 5);
    p->coords[0] = IVEC2(GetRandomValue(0, count-1), GetRandomValue(0, count-1));;
    p->color[0] = GetRandomValue(COLOR_BLUE, COLOR_COUNT-1);
    p->count = 1;
    for (i32 i = 1; i < count; i++) {
        iVec2 prev = p->coords[i-1];
        // find out the possible directions for this piece
        // by checking every direction against coords already in the pattern
        iVec2 possible_dirs[4];
        i32 num_dirs = 0;
        for (i32 j = 0; j < 4; j++) {
            if (!pattern_has_coord(p, ivec2_plus(DIRS[j], prev))) {
                possible_dirs[num_dirs++] = DIRS[j];
            }
        }
        if (num_dirs > 0) {
            p->coords[i] = ivec2_plus(prev, possible_dirs[GetRandomValue(0, num_dirs-1)]);
            p->color[i] = GetRandomValue(COLOR_BLUE, COLOR_COUNT-1);
            p->count++;
        } else {
            break;
        }
    }
    pattern_normalize(p);
}

