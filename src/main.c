#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include "screens.h"
#include "core/types.h"
#include "game.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const i32 RESOLUTION[] = { 320, 240 };
static const i32 SCALE = 2;

static bool on_transition = false;
static GameScreen current_screen = SCREEN_TITLE;
static i32 frameno = 0;

struct {
    float alpha;
    float fade_diff;
    GameScreen from_screen;
    GameScreen to_screen;
} trans = {
    .alpha = 0.f,
    .fade_diff = 0.f,
    .from_screen = SCREEN_UNKNOWN,
    .to_screen = SCREEN_UNKNOWN,
};

typedef struct ScreenInfo {
    void (*init)();
    void (*update)(f32, i32);
    void (*draw)(f32, i32);
    void (*unload)();
    int (*finish)();
} ScreenInfo;

ScreenInfo screen_table[] = {
    { .init = title_init,    .update = title_update,    .draw = title_draw,    .unload = title_unload,    .finish = title_finish,    },
    { .init = options_init,  .update = options_update,  .draw = options_draw,  .unload = options_unload,  .finish = options_finish,  },
    { .init = game_init, .update = game_update, .draw = game_draw, .unload = game_unload, .finish = game_finish, },
};

RenderTexture2D render_texture;

void iterate(void *arg);

int main(void)
{
    InitWindow(RESOLUTION[0] * SCALE, RESOLUTION[1] * SCALE, "raylib game template");
    InitAudioDevice();

    render_texture = LoadRenderTexture(RESOLUTION[0], RESOLUTION[1]);

    current_screen = SCREEN_TITLE;
    screen_table[current_screen].init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(iterate, &game_state, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        iterate(NULL);
    }
#endif

    screen_table[current_screen].unload();

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// Request transition to next screen
static void transition_to_screen(int screen)
{
    on_transition = true;
    trans.fade_diff = 0.05f;
    trans.from_screen = current_screen;
    trans.to_screen = screen;
    trans.alpha = 0.0f;
}

// Update transition effect (fade-in, fade-out)
static void update_transition(void)
{
    trans.alpha += trans.fade_diff;

    if (trans.alpha > 1.f) {
        trans.alpha = 1.f;
        // Unload current screen
        screen_table[trans.from_screen].unload();
        screen_table[trans.to_screen].init();
        current_screen = trans.to_screen;
        // Activate fade out effect to next loaded screen
        trans.fade_diff = -0.02f;
    }

    if (trans.alpha < 0.f) {
        on_transition = false;
        trans.alpha = 0.f;
        trans.fade_diff = 0.f;
        trans.from_screen = SCREEN_UNKNOWN;
        trans.to_screen = SCREEN_UNKNOWN;
    }
}

void iterate(void *arg)
{
    const f32 dt = GetFrameTime();
    frameno++;
    if (!on_transition) {
        screen_table[current_screen].update(dt, frameno);
        int res = screen_table[current_screen].finish();
        if (res != SCREEN_UNKNOWN) {
            transition_to_screen(res);
        }
    } else {
        update_transition();
    }

    BeginDrawing();
    BeginTextureMode(render_texture);
    ClearBackground(BLACK);

    screen_table[current_screen].draw(dt, frameno);

    // Draw full screen rectangle in front of everything
    if (on_transition) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, trans.alpha));
    }

    EndTextureMode();

    DrawTexturePro(
        render_texture.texture,
        rec(vec2(0, 0), vec2(RESOLUTION[0], -RESOLUTION[1])),
        rec(vec2(0, 0), Vector2Scale(vec2(RESOLUTION[0], RESOLUTION[1]), SCALE)),
        vec2(0, 0), 0, WHITE
    );

    EndDrawing();
}
