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

static inline Vector2 vec(float x, float y) { return (Vector2) { .x = x, .y = y }; }
