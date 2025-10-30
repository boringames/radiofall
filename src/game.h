#pragma once

#include "main.h"

#define GRID_WIDTH 8
#define GRID_HEIGHT 13

typedef enum GridColor {
    COLOR_EMPTY,
    COLOR_BLUE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_COUNT,
} GridColor;

void game_load();
void game_unload();
void game_enter();
void game_update(f32 dt, i32 frame);
void game_draw(f32 dt, i32 frame);
GameScreen game_exit();

