#include "pattern.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include "util.h"

QUEUE_DEFINE(Pattern, PatternBuffer, pattbuf, 32)
VECTOR_DEFINE(PatternVector, Pattern, pattvec)

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
    iVec2 min = pattern_min(p);
    for (i32 i = 0; i < p->count; i++) {
        p->coords[i].x -= min.x;
        p->coords[i].y -= min.y;
    }
}

iVec2 pattern_min(Pattern *p)
{
    i32 miny = INT_MAX;
    i32 minx = INT_MAX;
    for (i32 i = 0; i < p->count; i++) {
        miny = MIN(p->coords[i].y, miny);
        minx = MIN(p->coords[i].x, minx);
    }
    return IVEC2(minx, miny);
}

iVec2 pattern_max(Pattern *p)
{
    i32 maxy = INT_MIN;
    i32 maxx = INT_MIN;
    for (i32 i = 0; i < p->count; i++) {
        maxy = MAX(p->coords[i].y, maxy);
        maxx = MAX(p->coords[i].x, maxx);
    }
    return IVEC2(maxx, maxy);
}


Vector2 pattern_size(Pattern *p, float block_size)
{
    iVec2 max = pattern_max(p);
    return Vector2Scale(vec2(max.x + 1, max.y + 1), block_size);
}

iVec2 pattern_origin(Pattern *p)
{
    iVec2 m = pattern_max(p);
    return IVEC2(m.x/2, m.y/2);
}

void pattern_rotate(Pattern *p, bool ccw)
{
    iVec2 orig = pattern_origin(p);
    for (i32 i = 0; i < p->count; i++) {
        Vector2 v = Vector2Rotate(
            Vector2Subtract(as_vec2(orig), as_vec2(p->coords[i])),
            M_PI/2 * (ccw ? 1 : -1)
        );
        p->coords[i] = IVEC2(round(v.x), round(v.y));
    }
    orig = pattern_origin(p);
    pattern_normalize(p);
    for (i32 i = 0; i < p->count; i++) {
        Vector2 v = Vector2Add(as_vec2(p->coords[i]), as_vec2(orig));
        p->coords[i] = IVEC2(round(v.x), round(v.y));
    }
    pattern_normalize(p);
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

Pattern *pattern_dup(Pattern *p)
{
    Pattern *q = malloc(sizeof(Pattern));
    memcpy(q, p, sizeof(Pattern));
    return q;
}
