#pragma once

#include "util.h"
#include "queue.h"

#define ANIMATIONS_POOL_MAX 20

typedef void (*AnimationFunc)(void *context, f32 dt, i32 rel_frameno);

typedef struct {
    AnimationFunc animation;
    i32 for_frames;
    i32 cur_frame;

    void *data;
} _Animation;

typedef struct {
    _Animation animations[ANIMATIONS_POOL_MAX];
    i32 count;
} APool;

i32 apool_add(AnimationFunc anim, i32 for_frames, void *data, u64 size);
void apool_remove(i32 idx);
void apool_update(f32 dt);
