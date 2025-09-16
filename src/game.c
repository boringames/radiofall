#include <raylib.h>
#include <math.h>
#include "core/types.h"
#include "core/log.h"
#include "screens.h"

#define GRID_WIDTH 8
#define GRID_HEIGHT 12
#define GRID_CELL_SIDE 30

#define GAME_STEP_SPAN_SECS 1.0
#define GRID_VALID_PATTERN(p) (p.count >= 4)

#define _IN_GRID(i, j) (i >= 0 && i < GRID_HEIGHT && j >= 0 && j < GRID_WIDTH)

enum {
    GRID_CELL_COLOR_START,
    GRID_CELL_BLUE,
    GRID_CELL_RED,
    GRID_CELL_GREEN,
    GRID_CELL_YELLOW,
    GRID_CELL_COLOR_COUNT,
};

typedef struct {
    i32 grid[GRID_HEIGHT][GRID_WIDTH];
    i32 grid_pos[2];
} GameState;

GameState game_state;

static bool game_step(GameState *g);
static Color grid_cell_color(u8 c);

typedef struct {
    i32 row;
    i32 col;
} Cell;

typedef struct {
    i32 color;
    i32 count;
    Cell cells[GRID_HEIGHT * GRID_WIDTH];
} Pattern;

i32 _dirs[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
bool _grid_v[GRID_HEIGHT][GRID_WIDTH] = {0};

static void _find_pattern(GameState *g, i32 i, i32 j, i32 color, Pattern *pattern) {
    if ((!_IN_GRID(i, j)) || _grid_v[i][j] || (g->grid[i][j] != color)) return;
    _grid_v[i][j] = true;
    pattern->cells[pattern->count].row = i;
    pattern->cells[pattern->count].col = j;
    pattern->count++;
    for (i32 k=0; k<4; k++)
        _find_pattern(g, i + _dirs[k][0], j + _dirs[k][1], color, pattern);
}

static Pattern grid_find_pattern(GameState *g, i32 i, i32 j, i32 color) {
    for (i32 i = 0; i < GRID_HEIGHT; i++)
        for (i32 j = 0; j < GRID_WIDTH; j++)
            _grid_v[i][j] = false;

    Pattern p;
    p.count = 0;
    _find_pattern(g, i, j, color, &p);
    return p;
}

void game_init() {
    for (i32 i=0; i<GRID_HEIGHT; i++) {
        for (i32 j=0; j<GRID_WIDTH; j++) {
            if (i > GRID_HEIGHT - 4) {
                game_state.grid[i][j] = GetRandomValue(GRID_CELL_COLOR_START + 1, GRID_CELL_COLOR_COUNT - 1);
            } else if (i > GRID_HEIGHT - 6 && i <= GRID_HEIGHT - 4) {
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

    for (i32 i = 0; i < GRID_HEIGHT; i++) {
        for (i32 j = 0; j < GRID_WIDTH; j++) {
            if (game_state.grid[i][j]) {
                DrawRectangle(
                        GRID_CELL_SIDE * j + grid_x,
                        GRID_CELL_SIDE * i + grid_y,
                        GRID_CELL_SIDE,
                        GRID_CELL_SIDE,
                        grid_cell_color(game_state.grid[i][j])
                        );
            }
            DrawRectangleLines(
                    GRID_CELL_SIDE * j + grid_x,
                    GRID_CELL_SIDE * i + grid_y,
                    GRID_CELL_SIDE,
                    GRID_CELL_SIDE,
                    WHITE
                    );
        }
    }
}

static void grid_sweep(GameState *g) {
    for (i32 i = 0; i < GRID_HEIGHT; i++) {
        for (i32 j = 0; j < GRID_WIDTH; j++) {
            for (i32 pc=GRID_CELL_COLOR_START+1; pc<GRID_CELL_COLOR_COUNT; pc++) {
                Pattern p = grid_find_pattern(g, i, j, pc);
                if (GRID_VALID_PATTERN(p)) {
                    for (i32 k=0; k<p.count; k++)
                        g->grid[p.cells[k].row][p.cells[k].col] = 0;
                }
            }
        }
    }
}

void game_update(f32 dt, i32 frame) {
    if (frame % 16 == 0) {
        bool settled = game_step(&game_state);
        if (settled)
            grid_sweep(&game_state);
    }

    if (IsKeyPressed(KEY_R)) {
        game_init();
    }
}

static bool game_step(GameState *g) {
    bool settled = true;
    for (i32 i=GRID_HEIGHT-2; i>0; i--) {
        for (i32 j=0; j<GRID_WIDTH; j++) {
            if (g->grid[i+1][j] == 0 && g->grid[i][j] != 0) {
                g->grid[i+1][j] = g->grid[i][j];
                g->grid[i][j] = 0;
                settled = false;
            }
        }
    }
    return settled;
}

static Color grid_cell_color(u8 c) {
    switch (c)
    {
    case GRID_CELL_BLUE:
    return BLUE;
    case GRID_CELL_RED:
    return RED;
    case GRID_CELL_GREEN:
    return GREEN;
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
