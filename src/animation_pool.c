#include "animation_pool.h"

#include "core/types.h"
#include "raylib.h"
#include "util.h"

APool pool = {0};

static void MemCopy(void *dst, void *src, u64 size)
{
    u8 *d = (u8*)(dst); const u8 *s = (const u8*)(src);
    u64 i = 0;
    for (; i <= size - sizeof(uint64_t); i += sizeof(uint64_t))
        ((uint64_t*)d)[i/sizeof(uint64_t)] = ((const u64*)s)[i/sizeof(u64)];
    for (; i < size; i++)
        d[i] = s[i];
}

i32 apool_add(AnimationFunc anim, i32 for_frames, void *data, u64 size)
{
    if (pool.count >= ANIMATIONS_POOL_MAX) return -1;
    pool.animations[pool.count].animation = anim;
    pool.animations[pool.count].for_frames = for_frames;
    pool.animations[pool.count].cur_frame = 0;
    void *copybuf = NULL; if ((copybuf = MemAlloc(size))) {
        MemCopy(copybuf, data, size);
        pool.animations[pool.count].data = copybuf;
    } else {
        GAME_LOG_ERR("alloc of bytes %zu failed, got NULL MemAlloc ptr", size);
    }
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
