#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include "screens.h"
#include "core/types.h"
#include "game.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const int BASE_RESOLUTION[] = { 800, 450 };

static bool on_transition = false;
GameScreen current_screen = SCREEN_TITLE;

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
    void (*update)(f32);
    void (*draw)();
    void (*unload)();
    int (*finish)();
} ScreenInfo;

ScreenInfo screen_table[] = {
    { .init = title_init,    .update = title_update,    .draw = title_draw,    .unload = title_unload,    .finish = title_finish,    },
    { .init = options_init,  .update = options_update,  .draw = options_draw,  .unload = options_unload,  .finish = options_finish,  },
    { .init = game_init, .update = game_update, .draw = game_draw, .unload = game_unload, .finish = game_finish, },
};

void iterate(void *arg);

bool mode_debug = false;

int main(void)
{
    InitWindow(BASE_RESOLUTION[0], BASE_RESOLUTION[1], "raylib game template");
    InitAudioDevice();

    SetRandomSeed(GetTime());

    const char *_MODE_DEBUG = getenv("DEBUG");
    if (_MODE_DEBUG) {
        mode_debug = true;
    }

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
    if (!on_transition) {
        screen_table[current_screen].update(dt);
        int res = screen_table[current_screen].finish();
        if (res != SCREEN_UNKNOWN) {
            transition_to_screen(res);
        }
    } else {
        update_transition();
    }

    BeginDrawing();
    {
    ClearBackground(BLACK);

    screen_table[current_screen].draw();

    // Draw full screen rectangle in front of everything
    if (on_transition) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, trans.alpha));
    }

    }
    EndDrawing();
}
