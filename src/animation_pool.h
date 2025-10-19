#pragma once

#include "util.h"
#include "queue.h"

typedef bool (*AnimUpdateFunc)(void *context, f32 dt, i32 rel_frameno);

typedef struct {
    AnimUpdateFunc anim_update;
    i32 cur_frame;
    void *data;
} Animation;

void apool_add(Animation anim);
void apool_update(f32 dt);
void apool_free();
