#pragma once

#include "core/types.h"

typedef enum GameScreen {
    SCREEN_TITLE,
    SCREEN_OPTIONS,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING,
    SCREEN_UNKNOWN,
} GameScreen;

void title_init();
void title_update(f32 dt);
void title_draw();
void title_unload();
int title_finish();

void options_init();
void options_update(f32 dt);
void options_draw();
void options_unload();
int options_finish();

void game_init();
void game_update(f32 dt);
void game_draw();
void game_unload();
int game_finish();

void ending_init();
void ending_update(f32 dt);
void ending_draw();
void ending_unload();
int ending_finish();
