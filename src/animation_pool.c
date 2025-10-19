#include "animation_pool.h"

#include <string.h>
#include <raylib.h>
#include "util.h"

APool pool = {0};

i32 apool_add(AnimationFunc anim, i32 for_frames, void *data)
{
    if (pool.count >= ANIMATIONS_POOL_MAX) return -1;
    pool.animations[pool.count].animation = anim;
    pool.animations[pool.count].for_frames = for_frames;
    pool.animations[pool.count].cur_frame = 0;
    pool.animations[pool.count].data = data;
    return pool.count++;
}

void apool_remove(i32 idx)
{
    if (idx < 0 || idx >= pool.count) {
        return;
    }
    MemFree(pool.animations[idx].data);
    for (i32 i = idx; i < pool.count - 1; i++) {
        pool.animations[i] = pool.animations[i + 1];
    }
    pool.count--;
}

void apool_update(f32 dt)
{
    for (i32 i = 0; i < pool.count; i++) {
        if (pool.animations[i].for_frames > 0) {
            pool.animations[i].animation(
                pool.animations[i].data,
                dt, pool.animations[i].cur_frame++
            );
            if (pool.animations[i].cur_frame == pool.animations[i].for_frames) {
                apool_remove(i);
                i--;
            }
        }
    }
}
