#include <raylib.h>
#include "core/types.h"
#include "core/log.h"
#include "screens.h"

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

GameState game_state;

static void game_step(GameState *g);
static Color grid_cell_color(u8 c);

#define GAME_STEP_SPAN_SECS 1.0

void game_init() {
    for (i32 i=0; i<GRID_WIDTH; i++) {
        for (i32 j=0; j<GRID_HEIGHT; j++) {
            if (j > GRID_HEIGHT - 4) {
                game_state.grid[i][j] = GetRandomValue(GRID_CELL_COLOR_START + 1, GRID_CELL_COLOR_COUNT - 1);
            } else if (j > GRID_HEIGHT - 6 && j <= GRID_HEIGHT - 4) {
                game_state.grid[i][j] = GetRandomValue(0, GRID_CELL_COLOR_COUNT - 1);
            } else {
                game_state.grid[i][j] = 0;
            }
        }
    }

    // rob: do a couple of steps just to clear out empty spots lol
    game_step(&game_state);
    game_step(&game_state);
}

void game_draw() {
    int resolution[] = {
        GetRenderWidth(),
        GetRenderHeight(),
    };
    i32 grid_x = (resolution[0] / 2) - ((GRID_WIDTH * GRID_CELL_SIDE) / 2);
    i32 grid_y = (resolution[1]) - (GRID_HEIGHT * GRID_CELL_SIDE);

    for (i32 i = 0; i < GRID_WIDTH; i++) {
        for (i32 j = 0; j < GRID_HEIGHT; j++) {
            if (game_state.grid[i][j]) {
                DrawRectangle(
                        GRID_CELL_SIDE * i + grid_x,
                        GRID_CELL_SIDE * j + grid_y,
                        GRID_CELL_SIDE,
                        GRID_CELL_SIDE,
                        grid_cell_color(game_state.grid[i][j])
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
void game_update(f32 dt) {
    elaps += dt;
    if (elaps > GAME_STEP_SPAN_SECS) {
        game_step(&game_state);
        elaps = 0;
    }
    if (IsKeyPressed(KEY_R)) {
        game_init(&game_state);
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

void game_unload()
{
}

int game_finish()
{
    return SCREEN_UNKNOWN;
}
