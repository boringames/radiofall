#include "animation_pool.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <assert.h>
#include "util.h"
#include "vector.h"

VECTOR_DECLARE(APool, Animation, anim_pool)
VECTOR_DEFINE(APool, Animation, anim_pool)

APool pool = VECTOR_INIT();

void apool_add(Animation anim)
{
    anim_pool_add(&pool, anim);
}

void apool_update(f32 dt)
{
    for (ptrdiff_t i = 0; i < pool.size; i++) {
        if (pool.data[i].anim_update(pool.data[i].data, dt, pool.data[i].cur_frame++)) {
            anim_pool_remove_i(&pool, i);
            i--;
        }
    }
}

void apool_free()
{
    for (ptrdiff_t i = 0; i < pool.size; i++) {
        free(pool.data[i].data);
    }
    anim_pool_free(&pool);
}

ptrdiff_t apool_find_type_count(int type)
{
    ptrdiff_t count = 0;
    for (ptrdiff_t i = 0; i < pool.size; i++) {
        if (pool.data[i].type == type) {
            count++;
        }
    }
    return count;
}
