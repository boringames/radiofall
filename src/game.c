#include <raylib.h>
#include <math.h>
#include "core/types.h"
#include "core/log.h"
#include "screens.h"
#include "queue.h"

#define GRID_WIDTH 8
#define GRID_HEIGHT 12
#define GRID_CELL_SIDE 32

#define IN_GRID(v) (v.y >= 0 && v.y < GRID_HEIGHT && v.x >= 0 && v.x < GRID_WIDTH)

typedef enum GridColor {
    COLOR_EMPTY,
    COLOR_BLUE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_COUNT,
} GridColor;

struct {
    GridColor colors[GRID_HEIGHT][GRID_WIDTH];
    bool visited[GRID_HEIGHT][GRID_WIDTH];
} grid;

typedef struct {
    i32 count;
    iVec2 coords[GRID_HEIGHT * GRID_WIDTH];
} Pattern;

QUEUE_DECLARE(Pattern, PatternBuffer, pattbuf)
QUEUE_DEFINE(Pattern, PatternBuffer, pattbuf)

PatternBuffer pattern_buffer;

static const iVec2 dirs[4] = { IVEC2(1, 0), IVEC2(-1, 0), IVEC2(0, 1), IVEC2(0, -1) };

static void find_pattern(iVec2 pos, GridColor color, Pattern *pattern) {
    grid.visited[pos.y][pos.x] = true;
    pattern->coords[pattern->count] = pos;
    pattern->count++;
    for (i32 i = 0; i < 4; i++) {
        iVec2 neighbor = ivec2_plus(pos, dirs[i]);
        if (!grid.visited[neighbor.y][neighbor.x] && IN_GRID(neighbor) && grid.colors[neighbor.y][neighbor.x] == color) {
            find_pattern(neighbor, color, pattern);
        }
    }
}

void game_init() {
    for (i32 y = GRID_HEIGHT-1; y > 0; y--) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            grid.colors[y][x] =
                y > 8                                       ? GetRandomValue(COLOR_BLUE,  COLOR_COUNT - 1)
              : y > 6 && grid.colors[y+1][x] != COLOR_EMPTY ? GetRandomValue(COLOR_EMPTY, COLOR_COUNT - 1)
              :                                               COLOR_EMPTY;
        }
    }

    pattbuf_init(&pattern_buffer);
}

static Color grid_cell_color(u8 c) {
    switch (c)
    {
    case COLOR_BLUE:
    return BLUE;
    case COLOR_RED:
    return RED;
    case COLOR_GREEN:
    return GREEN;
    case COLOR_YELLOW:
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
            if (grid.colors[i][j]) {
                DrawRectangle(
                        GRID_CELL_SIDE * j + grid_x,
                        GRID_CELL_SIDE * i + grid_y,
                        GRID_CELL_SIDE,
                        GRID_CELL_SIDE,
                        grid_cell_color(grid.colors[i][j])
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
    for (i32 i = 0; i < GRID_HEIGHT; i++)
        for (i32 j = 0; j < GRID_WIDTH; j++)
            grid.visited[i][j] = false;

    for (i32 i = 0; i < GRID_HEIGHT; i++) {
        for (i32 j = 0; j < GRID_WIDTH; j++) {
            if (grid.colors[i][j] == COLOR_EMPTY || grid.visited[i][j]) {
                continue;
            }

            Pattern *p = pattbuf_enqueue_ptr(&pattern_buffer);
            if (!p) {
                continue;
            }

            p->count = 0;
            find_pattern(IVEC2(j, i), grid.colors[i][j], p);
            if (p->count < 4) {
                pattbuf_dequeue(&pattern_buffer, NULL);
            } else {
                for (i32 k=0; k<p->count; k++) {
                    grid.colors[p->coords[k].y][p->coords[k].x] = COLOR_EMPTY;
                }
            }
        }
    }
}

void game_update(f32 dt, i32 frame) {
    if (frame % 16 == 0) {
        bool settled = true;
        for (i32 i=GRID_HEIGHT-2; i>0; i--) {
            for (i32 j=0; j<GRID_WIDTH; j++) {
                if (grid.colors[i+1][j] == 0 && grid.colors[i][j] != 0) {
                    grid.colors[i+1][j] = grid.colors[i][j];
                    grid.colors[i][j] = 0;
                    settled = false;
                }
            }
        }
        if (settled) {
            grid_sweep();
        }
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
