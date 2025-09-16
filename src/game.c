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

#define IN_GRID(i, j) (i >= 0 && i < GRID_HEIGHT && j >= 0 && j < GRID_WIDTH)

enum {
    GRID_CELL_EMPTY,
    GRID_CELL_BLUE,
    GRID_CELL_RED,
    GRID_CELL_GREEN,
    GRID_CELL_YELLOW,
    GRID_CELL_COLOR_COUNT,
};

typedef struct {
    i32 color;
    i32 count;
    iVec2 coords[GRID_HEIGHT * GRID_WIDTH];
} Pattern;

i32 grid[GRID_HEIGHT][GRID_WIDTH];

const iVec2 dirs[4] = { IVEC2(1, 0), IVEC2(-1, 0), IVEC2(0, 1), IVEC2(0, -1) };

bool _grid_v[GRID_HEIGHT][GRID_WIDTH] = {0};

static void find_pattern(i32 i, i32 j, i32 color, Pattern *pattern) {
    if (!IN_GRID(i, j) || _grid_v[i][j] || grid[i][j] != color) {
        return;
    }
    _grid_v[i][j] = true;
    pattern->coords[pattern->count] = ivec2(i, j);
    pattern->count++;
    for (i32 k=0; k<4; k++) {
        iVec2 newdir = ivec2_plus(ivec2(i, j), dirs[k]);
        find_pattern(newdir.x, newdir.y, color, pattern);
    }
}

static Pattern grid_find_pattern(i32 i, i32 j, i32 color) {
    for (i32 i = 0; i < GRID_HEIGHT; i++)
        for (i32 j = 0; j < GRID_WIDTH; j++)
            _grid_v[i][j] = false;

    Pattern p;
    p.count = 0;
    find_pattern(i, j, color, &p);
    return p;
}

void game_init() {
    for (i32 y = GRID_HEIGHT-1; y > 0; y--) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            grid[y][x] = y > 8                                    ? GetRandomValue(GRID_CELL_BLUE,  GRID_CELL_COLOR_COUNT - 1)
                       : y > 6 && grid[y+1][x] != GRID_CELL_EMPTY ? GetRandomValue(GRID_CELL_EMPTY, GRID_CELL_COLOR_COUNT - 1)
                       :                                            GRID_CELL_EMPTY;
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
    case GRID_CELL_GREEN:
    return GREEN;
    case GRID_CELL_YELLOW:
    return YELLOW;
    default:
        GAME_LOG_ERR("no color matched, got value %d\n", c);
        return BLACK;
    }
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
            if (grid[i][j]) {
                DrawRectangle(
                        GRID_CELL_SIDE * j + grid_x,
                        GRID_CELL_SIDE * i + grid_y,
                        GRID_CELL_SIDE,
                        GRID_CELL_SIDE,
                        grid_cell_color(grid[i][j])
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

static void grid_sweep() {
    for (i32 i = 0; i < GRID_HEIGHT; i++) {
        for (i32 j = 0; j < GRID_WIDTH; j++) {
            for (i32 pc = GRID_CELL_BLUE; pc<GRID_CELL_COLOR_COUNT; pc++) {
                Pattern p = grid_find_pattern(i, j, pc);
                if (GRID_VALID_PATTERN(p)) {
                    for (i32 k=0; k<p.count; k++)
                        grid[p.coords[k].x][p.coords[k].y] = 0;
                }
            }
        }
    }
}

void game_update(f32 dt, i32 frame) {
    if (frame % 16 == 0) {
        /*
        bool settled = true;
        for (i32 i=GRID_HEIGHT-2; i>0; i--) {
            for (i32 j=0; j<GRID_WIDTH; j++) {
                if (grid[i+1][j] == 0 && grid[i][j] != 0) {
                    grid[i+1][j] = grid[i][j];
                    grid[i][j] = 0;
                    settled = false;
                }
            }
        }
        if (settled)
            grid_sweep();
            */
    }

    if (IsKeyPressed(KEY_R)) {
        game_init();
    }
}

void game_unload()
{
}

int game_finish()
{
    return SCREEN_UNKNOWN;
}
