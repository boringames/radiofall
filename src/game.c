#include "game.h"

#include <math.h>
#include <string.h>
#include <limits.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "core/types.h"
#include "screens.h"
#include "queue.h"
#include "loader.h"
#include "util.h"
#include "pattern.h"

#define IN_GRID(v) (v.y >= 0 && v.y < GRID_HEIGHT && v.x >= 0 && v.x < GRID_WIDTH)

struct {
    GridColor colors[GRID_HEIGHT][GRID_WIDTH];
    bool visited[GRID_HEIGHT][GRID_WIDTH];
} grid;

PatternBuffer pattern_buffer;

struct {
    iVec2 pos;
    Pattern patt;
} cur_piece;

enum {
    STATE_FALLING,
    STATE_SETTLING,
    STATE_END,
} cur_state;

Texture2D field_ui;
Texture2D field_ui_bg;
Texture2D blocks;

bool block_down = false;

// TextInsert for some reason has a bug
char *BetterTextInsert(const char *text, const char *insert, int position)
{
    int textLen = TextLength(text);
    int insertLen = TextLength(insert);

    char *result = (char *)malloc(textLen + insertLen + 1);

    memcpy(result, text, position);
    memcpy(result + position, insert, insertLen);
    memcpy(result + position + insertLen, text + position, textLen - position);

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
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

// check if (base_pos, pattern) is valid inside the grid
bool is_valid_pattern_pos(iVec2 base_pos, Pattern *p)
{
    for (i32 i = 0; i < p->count; i++) {
        iVec2 pos = ivec2_plus(base_pos, p->coords[i]);
        if (!IN_GRID(pos) || grid.colors[pos.y][pos.x] != COLOR_EMPTY) {
            return false;
        }
    }
    return true;
}

static void find_pattern(iVec2 pos, GridColor color, Pattern *pattern) {
    grid.visited[pos.y][pos.x] = true;
    pattern->coords[pattern->count] = pos;
    pattern->count++;
    for (i32 i = 0; i < 4; i++) {
        iVec2 neighbor = ivec2_plus(pos, DIRS[i]);
        if (!grid.visited[neighbor.y][neighbor.x] && IN_GRID(neighbor) && grid.colors[neighbor.y][neighbor.x] == color) {
            find_pattern(neighbor, color, pattern);
        }
    }
}

Texture2D load_texture(const char *path)
{
    const char *appdir = GetApplicationDirectory();
    char *realpath = BetterTextInsert(appdir, path, strlen(appdir));
    Texture2D t = LoadTexture(realpath);
    RL_FREE(realpath);
    return t;
}

void game_init() {
    field_ui = load_texture("resources/ui.png");
    field_ui_bg = load_texture("resources/ui_bg.png");
    blocks = load_texture("resources/blocks.png");

    // i32 nmaps = 0;
    // LoaderMap *maps = loader_load_maps("", &nmaps);
    // i32 map_id = GetRandomValue(0, nmaps);
    for (i32 y = 0; y < GRID_HEIGHT; y++)
        for (i32 x = 0; x < GRID_WIDTH; x++)
            grid.colors[y][x] = COLOR_EMPTY; // maps[map_id][y][x];

    cur_state = STATE_FALLING;
    pattbuf_init(&pattern_buffer);
    pattern_generate(&cur_piece.patt);
    cur_piece.pos = IVEC2(3, 0);
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
                pattbuf_dequeue_tail(&pattern_buffer, NULL);
            } else {
                for (i32 k=0; k<p->count; k++) {
                    grid.colors[p->coords[k].y][p->coords[k].x] = COLOR_EMPTY;
                }
                pattern_normalize(p);
            }
        }
    }
}

static void enter_settling_state()
{
    // copy current falling piece to grid
    for (i32 i = 0; i < cur_piece.patt.count; i++) {
        iVec2 pos = ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]);
        grid.colors[pos.y][pos.x] = cur_piece.patt.color[i];
    }

    grid_sweep();

    if (pattbuf_dequeue(&pattern_buffer, &cur_piece.patt)) {
        cur_piece.pos = IVEC2(3, 0);
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
            cur_piece.patt.color[i] = GetRandomValue(COLOR_BLUE, COLOR_COUNT - 1);
        }
    } else {
        cur_piece.pos = IVEC2(3, 0);
        pattern_generate(&cur_piece.patt);
    }

    if (!is_valid_pattern_pos(cur_piece.pos, &cur_piece.patt)) {
        cur_state = STATE_END;
    } else {
        cur_state = STATE_SETTLING;
    }
}

bool is_key_down(int key, i32 frame, i32 time)
{
    return IsKeyDown(key) && frame % time == 0;
}

void game_update(f32 dt, i32 frame) {
    switch (cur_state) {
    case STATE_FALLING:
        if (block_down && !IsKeyDown(KEY_DOWN)) {
            block_down = false;
        }

        if (IsKeyPressed(KEY_Z) != IsKeyPressed(KEY_X)) {
            Pattern p = { .count = cur_piece.patt.count };
            memcpy(p.coords, cur_piece.patt.coords, sizeof(iVec2) * p.count);
            pattern_rotate(&p, IsKeyPressed(KEY_Z));
            if (is_valid_pattern_pos(cur_piece.pos, &p)) {
                memcpy(cur_piece.patt.coords, p.coords, sizeof(iVec2) * p.count);
            }
        }

        i32 xdir = -is_key_down(KEY_LEFT, frame, 8) + is_key_down(KEY_RIGHT, frame, 8);
        if (!is_valid_pattern_pos(ivec2_plus(cur_piece.pos, IVEC2(xdir, 0)), &cur_piece.patt)) {
            xdir = 0;
        }
        i32 ydir = (!block_down && is_key_down(KEY_DOWN, frame, 3)) || frame % 64 == 0;
        if (!is_valid_pattern_pos(ivec2_plus(cur_piece.pos, IVEC2(xdir, ydir)), &cur_piece.patt)) {
            cur_piece.pos.x += xdir;
            enter_settling_state();
        } else {
            cur_piece.pos = ivec2_plus(cur_piece.pos, IVEC2(xdir, ydir));
        }
        break;
    case STATE_SETTLING:
        block_down = true;
        // do some animations
        cur_state = STATE_FALLING;
        break;
    default:
        break;
    }

    if (IsKeyPressed(KEY_R)) {
        game_init();
    }
}

void draw_block(Vector2 pos, GridColor color, i32 frame)
{
    DrawTextureRec(blocks,
        rec(vec2((color-1) * GRID_CELL_SIDE, frame * GRID_CELL_SIDE), vec2(GRID_CELL_SIDE, GRID_CELL_SIDE)),
        pos,
        WHITE
    );
}

i32 block_frameno = 0;
i32 block_frame_step = 1;

void game_draw(f32 dt, i32 frame) {
    Vector2 grid_pos = vec2(96, 16);

    if (frame % 16 == 0) {
        block_frameno += block_frame_step;
        if (block_frameno == 3 && block_frame_step == 1) block_frame_step *= -1;
        if (block_frameno == 0 && block_frame_step == -1) block_frame_step *= -1;
    }

    for (i32 i = 0; i < cur_piece.patt.count; i++) {
        Vector2 pos = as_vec2(ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]));
        draw_block(
            Vector2Add(Vector2Scale(pos, GRID_CELL_SIDE), grid_pos),
            cur_piece.patt.color[i],
            block_frameno
        );
    }

    for (i32 y = 0; y < GRID_HEIGHT; y++) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            if (grid.colors[y][x] != COLOR_EMPTY) {
                draw_block(
                    Vector2Add(Vector2Scale(vec2(x, y), GRID_CELL_SIDE), grid_pos),
                    grid.colors[y][x],
                    block_frameno
                );
            }
        }
    }

    DrawTexture(field_ui, 0, 0, WHITE);
}

void game_unload()
{
}

int game_finish()
{
    if (cur_state == STATE_END) {
        return SCREEN_TITLE;
    }
    return SCREEN_UNKNOWN;
}
