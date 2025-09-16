#include <math.h>
#include <limits.h>
#include <raylib.h>
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
    i32 color[GRID_HEIGHT * GRID_WIDTH];
} Pattern;

QUEUE_DECLARE(Pattern, PatternBuffer, pattbuf)
QUEUE_DEFINE(Pattern, PatternBuffer, pattbuf)

PatternBuffer pattern_buffer;

struct {
    iVec2 pos;
    Pattern patt;
} cur_piece;

enum {
    STATE_FALLING,
    STATE_SETTLING,
} cur_state;


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
    cur_piece.patt.count = 0;
    cur_state = STATE_SETTLING;
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

static void pattern_reset(Pattern *p)
{
    // find minimum y of coordinates into pattern
    i32 miny = INT_MAX;
    i32 minx = INT_MAX;
    // i32 maxx = INT_MIN;
    for (i32 i = 0; i < p->count; i++) {
        miny = MIN(p->coords[i].y, miny);
        minx = MIN(p->coords[i].x, minx);
        // maxx = MAX(p->coords[i].x, maxx);
    }
    // i32 base_x = (8 - (maxx - minx)) / 2;
    for (i32 i = 0; i < p->count; i++) {
        p->coords[i].y -= miny;
        p->coords[i].x -= minx;
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
                pattern_buffer.t = (pattern_buffer.t - 1) % 8;
            } else {
                for (i32 k=0; k<p->count; k++) {
                    grid.colors[p->coords[k].y][p->coords[k].x] = COLOR_EMPTY;
                }
                pattern_reset(p);
            }
        }
    }
}

void game_update(f32 dt, i32 frame) {
    switch (cur_state) {
    case STATE_FALLING:
        if (frame % 16 == 0) {
            bool settled = false;
            for (i32 i = 0; i < cur_piece.patt.count; i++) {
                iVec2 pos = ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]);
                if (grid.colors[pos.y+1][pos.x] != COLOR_EMPTY) {
                    settled = true;
                }
            }
            if (settled) {
                for (i32 i = 0; i < cur_piece.patt.count; i++) {
                    iVec2 pos = ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]);
                    grid.colors[pos.y][pos.x] = cur_piece.patt.color[i];
                }
                cur_state = STATE_SETTLING;
            } else {
                cur_piece.pos.y += 1;
            }
        }
        break;
    case STATE_SETTLING:
        grid_sweep();
        // bool settled = true;
        // for (i32 i=GRID_HEIGHT-2; i>0; i--) {
        //     for (i32 j=0; j<GRID_WIDTH; j++) {
        //         if (grid.colors[i+1][j] == 0 && grid.colors[i][j] != 0) {
        //             // grid.colors[i+1][j] = grid.colors[i][j];
        //             // grid.colors[i][j] = 0;
        //             settled = false;
        //         }
        //     }
        // }
        // if (settled) {
            if (pattbuf_dequeue(&pattern_buffer, &cur_piece.patt)) {
                cur_piece.pos = IVEC2(3, 0);
                for (i32 i = 0; i < cur_piece.patt.count; i++) {
                    cur_piece.patt.color[i] = GetRandomValue(COLOR_BLUE,  COLOR_COUNT - 1);
                }
            }
            cur_state = STATE_FALLING;
        // }
        break;
    }

    if (IsKeyPressed(KEY_R)) {
        game_init();
    }
}

void game_draw() {
    int resolution[] = {
        GetRenderWidth(),
        GetRenderHeight(),
    };
    i32 grid_x = (resolution[0] / 2) - ((GRID_WIDTH * GRID_CELL_SIDE) / 2);
    i32 grid_y = (resolution[1]) - (GRID_HEIGHT * GRID_CELL_SIDE);

    for (i32 i = 0; i < cur_piece.patt.count; i++) {
        iVec2 pos = ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]);
        DrawRectangle(
            pos.x * GRID_CELL_SIDE + grid_x,
            pos.y * GRID_CELL_SIDE + grid_y,
            GRID_CELL_SIDE, GRID_CELL_SIDE,
            grid_cell_color(cur_piece.patt.color[i])
        );
    }

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

void game_unload()
{
}

int game_finish()
{
    return SCREEN_UNKNOWN;
}
