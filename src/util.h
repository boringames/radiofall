#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <raylib.h>
#include <inttypes.h>

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

#define VEC2ZERO ((Vector2) { .x = 0.f, .y = 0.f })
static inline Vector2 vec2(float x, float y) { return (Vector2) { .x = x, .y = y }; }

typedef struct iVec2 {
    i32 x, y;
} iVec2;

#define IVEC2(a, b) ((iVec2) { .x = (a), .y = (b) })

static inline iVec2 ivec2_plus(iVec2 a, iVec2 b) { return (iVec2) { .x = a.x + b.x, .y = a.y + b.y }; }
static inline bool ivec2_eq(iVec2 a, iVec2 b) { return a.x == b.x && a.y == b.y; }
static inline Vector2 as_vec2(iVec2 v) { return (Vector2) { .x = (float) v.x, .y = (float) v.y }; }
static inline iVec2 ivec2_scale(iVec2 v, i32 x) { return (iVec2) { .x = v.x * x, .y = v.y * x }; }

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(x, a, b) MIN(b, MAX(a, x))

static inline Rectangle rec(Vector2 pos, Vector2 size) {
    return (Rectangle) {
        .x = pos.x, .y = pos.y,
        .width = size.x, .height = size.y
    };
}
#define COUNT_OF(a) (sizeof(a)/sizeof(a[0]))

#define GAME_LOG(STREAM, FMT, ...) \
        do{fprintf(STREAM, "[%s:%s:%d]: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);}while(0)
#define GAME_LOG_ERR(FMT, ...) \
        do{fprintf(stderr, "[%s:%s:%d][ERROR]: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);}while(0)

static const iVec2 DIRS[4] = { IVEC2(1, 0), IVEC2(-1, 0), IVEC2(0, 1), IVEC2(0, -1) };

// TextInsert for some reason has a bug
char *text_insert(const char *text, const char *insert, int position);
void draw_text_centered(const char *text, Vector2 position, int padding, Color color, Font font, float fontSize, float spacing);
Texture2D load_texture(const char *path);
Sound load_sound(const char *path);
Shader load_shader(const char *vs, const char *fs);
Font *load_font_sdf(const char *path, int baseSize, int *codepoints, int cp_count);

static inline void shader_setf(Shader shader, const char *name, float value)
{
    SetShaderValue(shader, GetShaderLocation(shader, name), &value, SHADER_UNIFORM_FLOAT);
}

static inline void shader_setv3(Shader shader, const char *name, Vector3 value)
{
    SetShaderValue(shader, GetShaderLocation(shader, name), &value, SHADER_UNIFORM_VEC3);
}

static inline Vector3 color_to_vec3(Color c)
{
    return (Vector3) { .x = c.r / 255.f, .y = c.g / 255.f, .z = c.b / 255.f };
}

static inline f32 lerp(f32 a, f32 b, f32 t)
{
    return (1 - t) * a + t * b;
}

#define DEFINE_DUP_FN(type, name) \
type *name##_dup(type *a) \
{ \
    type *b = malloc(sizeof(*a)); \
    memcpy(b, a, sizeof(*a)); \
    return b; \
} \

