#include "game.h"
#include "core/types.h"
#include "core/log.h"

static const int resolution[] = { 800, 450 };

static void game_step(GameState *g);
static Color grid_cell_color(u8 c);

#define GAME_STEP_SPAN_SECS 1.0

void game_init(GameState *g) {
    for (i32 i=0; i<GRID_WIDTH; i++) {
        for (i32 j=0; j<GRID_HEIGHT; j++) {
            if (j > GRID_HEIGHT - 4) {
                g->grid[i][j] = GetRandomValue(GRID_CELL_COLOR_START + 1, GRID_CELL_COLOR_COUNT - 1);
            } else if (j > GRID_HEIGHT - 6 && j <= GRID_HEIGHT - 4) {
                g->grid[i][j] = GetRandomValue(0, GRID_CELL_COLOR_COUNT - 1);
            } else {
                g->grid[i][j] = 0;
            }
        }
    }

    // rob: do a couple of steps just to clear out empty spots lol
    game_step(g);
    game_step(g);
}

void game_draw(GameState *g, f32 dt) {
    i32 grid_x = (resolution[0] / 2) - ((GRID_WIDTH * GRID_CELL_SIDE) / 2);
    i32 grid_y = (resolution[1]) - (GRID_HEIGHT * GRID_CELL_SIDE);

    for (i32 i = 0; i < GRID_WIDTH; i++) {
        for (i32 j = 0; j < GRID_HEIGHT; j++) {
            if (g->grid[i][j]) {
                DrawRectangle(
                        GRID_CELL_SIDE * i + grid_x,
                        GRID_CELL_SIDE * j + grid_y,
                        GRID_CELL_SIDE,
                        GRID_CELL_SIDE,
                        grid_cell_color(g->grid[i][j])
                        );
            }
            DrawRectangleLines(
                    GRID_CELL_SIDE * i + grid_x,
                    GRID_CELL_SIDE * j + grid_y,
                    GRID_CELL_SIDE,
                    GRID_CELL_SIDE,
                    WHITE
                    );
        }
    }
}

f32 elaps = 0;
void game_update(GameState *g, f32 dt) {
    elaps += dt;
    if (elaps > GAME_STEP_SPAN_SECS) {
        game_step(g); elaps = 0;
    }
}

static void game_step(GameState *g) {
    for (i32 j=GRID_HEIGHT-2; j>0; j--) {
        for (i32 i=0; i<GRID_WIDTH; i++) {
            if (g->grid[i][j+1] == 0) {
                g->grid[i][j+1] = g->grid[i][j];
                g->grid[i][j] = 0;
            }
        }
    }
}

static Color grid_cell_color(u8 c) {
    switch (c)
    {
    case GRID_CELL_BLUE:
    return BLUE;
    case GRID_CELL_RED:
    return RED;
    case GRID_CELL_YELLOW:
    return YELLOW;
    default:
        GAME_LOG_ERR("no color matched, got value %d\n", c);
        return BLACK;
    }
}