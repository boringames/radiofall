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
#include "util.h"
#include "pattern.h"
#include "animation_pool.h"

#define PATTERN_MATCH_MIN 3

#define IN_GRID(v) (v.y >= 0 && v.y < GRID_HEIGHT && v.x >= 0 && v.x < GRID_WIDTH)

struct {
    GridColor colors[GRID_HEIGHT][GRID_WIDTH];
    bool visited[GRID_HEIGHT][GRID_WIDTH];
} grid;

PatternBuffer pattern_buffer;
PatternBuffer matched_patterns;

static const Vector2 GRID_POS = {96, 16};
i32 block_frameno = 0;
i32 block_frame_step = 1;

i32 score = 0;
i32 total_matched = 0;

struct {
    iVec2 pos;
    Pattern patt;
} cur_piece;

enum {
    STATE_FALLING,
    STATE_SETTLING,
    STATE_SHOW_MATCHES,
    STATE_END,
} cur_state;

Texture2D field_ui;
Texture2D field_ui_bg;
Texture2D preview1;
Texture2D preview2;
Texture2D blocks;
Texture2D volume_img;

// true when the player has just positioned a piece
bool block_down = false;

// timer for each state; resets to zero on a state change
i32 state_timer = 0;

#define COOLDOWN_MAX 20

i32 volume_cooldown = 0;

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

void DrawTextCentered(const char *text, Vector2 position, int padding, Color color, Font font, float fontSize, float spacing)
{
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
    Vector2 drawPos = {
        position.x - (textSize.x + 2 * padding) / 2,
        position.y - (textSize.y + 2 * padding) / 2
    };
    DrawTextEx(font, text, drawPos, fontSize, spacing, color);
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

static bool grid_sweep() {
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
            if (p->count <= PATTERN_MATCH_MIN) {
                pattbuf_dequeue_tail(&pattern_buffer, NULL);
            } else {
                for (i32 k=0; k<p->count; k++) {
                    grid.colors[p->coords[k].y][p->coords[k].x] = COLOR_EMPTY;
                }
                for (i32 i = 0; i < p->count; i++) {
                    p->color[i] = GetRandomValue(COLOR_BLUE, COLOR_COUNT - 1);
                }
                pattbuf_enqueue(&matched_patterns, *p);
                pattern_normalize(p);
            }
        }
    }

    return pattbuf_size(&matched_patterns);
}

static void enter_falling_state()
{
    if (!pattbuf_dequeue(&pattern_buffer, &cur_piece.patt)) {
        pattern_generate(&cur_piece.patt);
    }
    cur_piece.pos = IVEC2(3, 0);

    if (!is_valid_pattern_pos(cur_piece.pos, &cur_piece.patt)) {
        cur_state = STATE_END;
    } else {
        cur_state = STATE_FALLING;
    }
    state_timer = 0;
}

static void exit_falling_state()
{
    // copy current falling piece to grid
    for (i32 i = 0; i < cur_piece.patt.count; i++) {
        iVec2 pos = ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i]);
        grid.colors[pos.y][pos.x] = cur_piece.patt.color[i];
    }
    cur_piece.pos = IVEC2(-100, -100);
    cur_state = STATE_SETTLING;
    state_timer = 0;
}

bool is_key_down(int key, i32 timer, i32 time)
{
    return IsKeyDown(key) && timer % time == 0;
}

void game_init() {
    field_ui = load_texture("resources/ui.png");
    field_ui_bg = load_texture("resources/ui_bg.png");
    preview1 = load_texture("resources/ui_preview1.png");
    preview2 = load_texture("resources/ui_preview2.png");
    blocks = load_texture("resources/blocks.png");
    volume_img = load_texture("resources/volume.png");

    // i32 nmaps = 0;
    // LoaderMap *maps = loader_load_maps("", &nmaps);
    // i32 map_id = GetRandomValue(0, nmaps);
    for (i32 y = 0; y < GRID_HEIGHT; y++)
        for (i32 x = 0; x < GRID_WIDTH; x++)
            grid.colors[y][x] = COLOR_EMPTY; // maps[map_id][y][x];

    pattbuf_init(&pattern_buffer);
    pattbuf_init(&matched_patterns);

    score = 0;
    volume_cooldown = 0;
    total_matched = 0;
    state_timer = 0;
    enter_falling_state();
}

void animate_score(void *context, f32 dt, i32 frameno) {
    MatchInfo *m = (MatchInfo *)(context);
    DrawText(TextFormat("+%d", m->pcount),
            (m->pos.x * GRID_CELL_SIDE) + GRID_POS.x + cos(frameno) ,
            (m->pos.y * GRID_CELL_SIDE) + GRID_POS.y + sin(frameno),
            12, WHITE);
}

void game_update(f32 dt, i32 frame) {
    state_timer++;

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

        i32 xdir = -is_key_down(KEY_LEFT, state_timer, 8) + is_key_down(KEY_RIGHT, state_timer, 8);
        if (!is_valid_pattern_pos(ivec2_plus(cur_piece.pos, IVEC2(xdir, 0)), &cur_piece.patt)) {
            xdir = 0;
        }
        i32 ydir = (!block_down && is_key_down(KEY_DOWN, state_timer, 3)) || state_timer % 64 == 0;
        if (!is_valid_pattern_pos(ivec2_plus(cur_piece.pos, IVEC2(xdir, ydir)), &cur_piece.patt)) {
            cur_piece.pos.x += xdir;
            exit_falling_state();
        } else {
            cur_piece.pos = ivec2_plus(cur_piece.pos, IVEC2(xdir, ydir));
        }

        if (frame % 64 == 0) {
            volume_cooldown = CLAMP(volume_cooldown + 1, 0, COOLDOWN_MAX);
            if (volume_cooldown == COOLDOWN_MAX) {
                cur_state = STATE_END;
            }
        }
        break;
    case STATE_SETTLING:
        block_down = true;

        if (state_timer % 6 == 0) {
            // push down by 1 all blocks in the grid
            // that don't have anything below them
            bool all_settled = true;
            for (i32 y = GRID_HEIGHT - 2; y >= 0; y--) {
                for (i32 x = 0; x < GRID_WIDTH; x++) {
                    if (grid.colors[y][x] != COLOR_EMPTY && grid.colors[y+1][x] == COLOR_EMPTY) {
                        grid.colors[y+1][x] = grid.colors[y][x];
                        grid.colors[y][x] = COLOR_EMPTY;
                        all_settled = false;
                    }
                }
            }

            if (all_settled) {
                if (grid_sweep()) {
                    cur_state = STATE_SHOW_MATCHES;
                    state_timer = 0;
                } else {
                    enter_falling_state();
                }
            }
        }
        break;

    case STATE_SHOW_MATCHES:
        i32 local_score = 0;
        i32 matched_count = 0;
        while (pattbuf_size(&matched_patterns) != 0) {
            Pattern out;
            if (pattbuf_dequeue(&matched_patterns, &out)) {
                matched_count++;
                local_score += out.count;
                volume_cooldown = CLAMP(volume_cooldown - out.count, 0, COOLDOWN_MAX);
                MatchInfo m;
                m.pcount = out.count;
                m.pos = pattern_min(&out);
                pattern_normalize(&out);
                iVec2 pattern_orig = pattern_origin(&out);
                m.pos.x += pattern_orig.x;
                m.pos.y += pattern_orig.y;
                apool_add(animate_score, 128, (void *)&m, sizeof(MatchInfo));
            }
        }
        score += local_score * matched_count;
        total_matched += matched_count;
        if (state_timer == 32) {
            cur_state = STATE_SETTLING;
            state_timer = 0;
        }
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
        rec(vec2((color-1) * GRID_CELL_SIDE, frame * GRID_CELL_SIDE),
            vec2(GRID_CELL_SIDE, GRID_CELL_SIDE)),
        pos, WHITE
    );
}

void draw_preview(size_t n, Vector2 where, Vector2 box_size, Texture2D bg, i32 frame)
{
    i32 bg_frame = ((frame / 8) % 2) * box_size.x;
    DrawTextureRec(bg, rec(vec2(bg_frame, 0), box_size), where, WHITE);
    if (pattbuf_size(&pattern_buffer) < n) {
        return;
    }
    Pattern *p = pattbuf_peek(&pattern_buffer, n);
    iVec2 max = pattern_max(p);
    Vector2 patt_size = Vector2Scale(vec2(max.x + 1, max.y + 1), GRID_CELL_SIDE);
    Vector2 base_pos = vec2(where.x + floorf((box_size.x - patt_size.x) * 0.5f),
                            where.y + floorf((box_size.y - patt_size.y) * 0.5f));
    for (i32 i = 0; i < p->count; i++) {
        Vector2 pos = Vector2Scale(as_vec2(p->coords[i]), GRID_CELL_SIDE);
        draw_block(Vector2Add(base_pos, pos), p->color[i], 0);
    }
}

const i32 frames[] = { 0, 1, 2, 3, 2, 1, };

void game_draw(f32 dt, i32 frame) {
    Vector2 grid_pos = vec2(96, 16);
    i32 bg_frame = ((frame / 8) % 2) * 128;
    DrawTextureRec(field_ui_bg, rec(vec2(bg_frame, 0), vec2(128, 208)), grid_pos, WHITE);

    i32 block_frameno = frames[(frame/64) % COUNT_OF(frames)];

    for (i32 i = 0; i < cur_piece.patt.count; i++) {
        Vector2 pos = Vector2Scale(as_vec2(ivec2_plus(cur_piece.pos, cur_piece.patt.coords[i])), GRID_CELL_SIDE);
        draw_block(Vector2Add(pos, grid_pos), cur_piece.patt.color[i], block_frameno);
    }

    draw_preview(0, vec2(7,  28), vec2(68, 47), preview1, frame);
    draw_preview(1, vec2(8, 165), vec2(64, 65), preview2, frame);

    for (i32 y = 0; y < GRID_HEIGHT; y++) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            if (grid.colors[y][x] != COLOR_EMPTY) {
                draw_block(
                        Vector2Add(Vector2Scale(vec2(x, y), GRID_CELL_SIDE), GRID_POS),
                        grid.colors[y][x],
                        block_frameno
                        );
            }
        }
    }

    apool_update(dt);

    DrawTexture(field_ui, 0, 0, WHITE);
    {
        DrawTextCentered(TextFormat("%d", score), (Vector2){280, 34}, 2, WHITE, GetFontDefault(), 12, 1);
        DrawTextCentered(TextFormat("%d", total_matched), (Vector2){280, 54}, 2, WHITE, GetFontDefault(), 12, 1);

        Vector2 volume_size = vec2(66 - volume_cooldown, 45 - volume_cooldown);
        DrawTexturePro(
            volume_img,
            rec(vec2(0, 0), volume_size),
            rec(vec2(247 + volume_cooldown, 174 + volume_cooldown), volume_size),
            vec2(0, 0), 0, WHITE
        );
    }
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
