#pragma once

#include <raylib.h>
#include "core/types.h"

#define GRID_WIDTH 8
#define GRID_HEIGHT 12
#define GRID_CELL_SIDE 30

enum {
    GRID_CELL_COLOR_START,
    GRID_CELL_BLUE,
    GRID_CELL_RED,
    GRID_CELL_YELLOW,
    GRID_CELL_COLOR_COUNT,
};

typedef struct {
    i32 grid[GRID_WIDTH][GRID_HEIGHT];
    i32 grid_pos[2];
} GameState;

void game_init(GameState *g);

void game_update(GameState *g, f32 dt);

void game_draw(GameState *g, f32 dt);
