#pragma once

#include "core/types.h"

typedef enum GameScreen {
    SCREEN_TITLE,
    SCREEN_OPTIONS,
    SCREEN_GAMEPLAY,
    SCREEN_UNKNOWN,
} GameScreen;

void title_init();
void title_update(f32 dt, i32 frame);
void title_draw();
void title_unload();
int title_finish();

void options_init();
void options_update(f32 dt, i32 frame);
void options_draw();
void options_unload();
int options_finish();

void game_init();
void game_update(f32 dt, i32 frame);
void game_draw();
void game_unload();
int game_finish();
