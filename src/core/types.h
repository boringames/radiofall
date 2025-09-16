#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <raylib.h>

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

static inline Vector2 vec2(float x, float y) { return (Vector2) { .x = x, .y = y }; }

typedef struct iVec2 {
    i32 x, y;
} iVec2;

#define IVEC2(a, b) ((iVec2) { .x = (a), .y = (b) })

static inline iVec2 ivec2(i32 x, i32 y) { return (iVec2) { .x = x, .y = y }; }
static inline Vector2 as_vec2(iVec2 v) { return (Vector2) { .x = v.x, .y = v.y }; }
static inline iVec2 ivec2_plus(iVec2 a, iVec2 b) { return (iVec2) { .x = a.x + b.x, .y = a.y + b.y }; }

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

