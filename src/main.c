#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#include "screens.h"

#include "core/types.h"
#include "game.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static const int resolution[] = { 800, 450 };

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
    void (*update)();
    void (*draw)();
    void (*unload)();
    int (*finish)();
} ScreenInfo;

ScreenInfo screen_table[] = {
    { .init = title_init,    .update = title_update,    .draw = title_draw,    .unload = title_unload,    .finish = title_finish,    },
    { .init = options_init,  .update = options_update,  .draw = options_draw,  .unload = options_unload,  .finish = options_finish,  },
    { .init = gameplay_init, .update = gameplay_update, .draw = gameplay_draw, .unload = gameplay_unload, .finish = gameplay_finish, },
    { .init = ending_init,   .update = ending_update,   .draw = ending_draw,   .unload = ending_unload,   .finish = ending_finish,   },
};

void iterate(GameState *g);

b8 MODE_DEBUG = FALSE;

int main(void)
{
    InitWindow(resolution[0], resolution[1], "raylib game template");
    InitAudioDevice();

    SetRandomSeed(GetTime());

    GameState game_state;
    game_init(&game_state);

    const char *_MODE_DEBUG = getenv("DEBUG");
    if (_MODE_DEBUG) {
        MODE_DEBUG = TRUE;
    }

    current_screen = SCREEN_TITLE;
    screen_table[current_screen].init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(iterate, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        iterate(&game_state);
        if (IsKeyPressed(KEY_R)) {
            game_init(&game_state);
        }
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

static void iterate(GameState *g)
{
    const f32 dt = GetFrameTime();
    if (!on_transition) {
        screen_table[current_screen].update();
        int res = screen_table[current_screen].finish();
        if (res != SCREEN_UNKNOWN) {
            transition_to_screen(res);
        }
    } else {
        update_transition();
    }

    game_update(g, dt);

    BeginDrawing();
    {
    ClearBackground(BLACK);

    screen_table[current_screen].draw();

    game_draw(g, dt);

    // Draw full screen rectangle in front of everything
    if (on_transition) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, trans.alpha));
    }

    }
    EndDrawing();
}
