#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <raymath.h>
#include "util.h"
#include "data.h"
#include "game.h"
#include "title.h"
#include "main.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const i32 SCALE = 4;

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
    void (*enter)();
    void (*update)(f32, i32);
    void (*draw)(f32, i32);
    GameScreen (*exit)();
} ScreenInfo;

ScreenInfo screen_table[] = {
    [SCREEN_TITLE] = { .enter = title_enter, .update = title_update, .draw = title_draw, .exit = title_exit, },
    [SCREEN_GAMEPLAY] = { .enter = game_enter,  .update = game_update,  .draw = game_draw,  .exit = game_exit,  },
};

RenderTexture2D render_texture;

void iterate(void *arg);

int main(void)
{
    InitWindow(RESOLUTION[0] * SCALE, RESOLUTION[1] * SCALE, "raylib game template");
    InitAudioDevice();

    if (!data_init()) {
        return 1;
    }

    render_texture = LoadRenderTexture(RESOLUTION[0], RESOLUTION[1]);

    title_load();
    game_load();

    current_screen = SCREEN_TITLE;
    screen_table[current_screen].enter();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop_arg(iterate, &game_state, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        iterate(NULL);
    }
#endif

    game_unload();
    title_unload();

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
        screen_table[trans.to_screen].enter();
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
        int res = screen_table[current_screen].exit();
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
