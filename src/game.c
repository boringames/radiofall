#include "game.h"

#include <raylib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include "queue.h"
#include "util.h"
#include "pattern.h"
#include "animation_pool.h"
#include "vector.h"
#include "llist.h"

#define PATTERN_MATCH_MIN 3

#define IN_GRID(v) (v.y >= 0 && v.y < GRID_HEIGHT && v.x >= 0 && v.x < GRID_WIDTH)

struct {
    GridColor colors[GRID_HEIGHT][GRID_WIDTH];
    bool visited[GRID_HEIGHT][GRID_WIDTH];
} grid;

typedef struct {
    iVec2 pos; // origin of the pattern match and just "sweeped"
    i32 pcount;
} MatchInfo;

PatternBuffer pattern_buffer;

static const Vector2 GRID_POS = {96, 16};
i32 block_frameno = 0;
i32 block_frame_step = 1;

i32 score = 0;
i32 total_matched = 0;

struct {
    Vector2 pos;
    Pattern patt;
} cur_piece;

typedef struct {
    GridColor color;
    Vector2 pos;
    float vel;
} FallingBlock;

LLIST_DECLARE(FallingList, FallingBlock, falling_list)
LLIST_DEFINE(FallingList, FallingBlock, falling_list)

FallingList *falling_blocks;

enum {
    STATE_FALLING,
    STATE_SETTLING,
    STATE_END,
} cur_state;

Texture2D field_ui;
Texture2D field_ui_bg;
Texture2D preview1;
Texture2D preview2;
Texture2D blocks;
Texture2D volume_img;

Sound match_sfx;
Sound rotate_sfx;

// true when the player has just positioned a piece
bool block_down = false;

// timer for each state; resets to zero on a state change
i32 state_timer = 0;

// volume stuff
#define COOLDOWN_MAX 20

i32 volume_cooldown = 0;

// key input buffers
// we check input for these keys each frame, but their action
// is delayed by a few frames so that they're slowed down
#define INPUT_LEFT 0
#define INPUT_RIGHT 1
#define INPUT_DOWN 2

bool input_buffers[2];

struct {
    Pattern pattern;
    bool in;
    i32 init_timer;
} rotation;

int raylib_key_to_input_key(KeyboardKey key)
{
    return key == KEY_LEFT  ? INPUT_LEFT
         : key == KEY_RIGHT ? INPUT_RIGHT
         : key == KEY_DOWN  ? INPUT_DOWN
         : 0;
}

iVec2 convert_base_pos(Vector2 p)
{
    return IVEC2(p.x / 16.f, p.y / 16.f);
}

// check if (base_pos, pattern) is valid inside the grid
bool is_valid_pattern_pos(Vector2 base_pos, Pattern *p)
{
    // check out any possible grid position the pattern may be in
    // this mostly means checking at base_pos.y and base_pos.y + 15
    iVec2 bps[] = {
        convert_base_pos(base_pos),
        convert_base_pos(Vector2Add(base_pos, vec2(0, 15))),
    };
    for (size_t j = 0; j < COUNT_OF(bps); j++) {
        for (i32 i = 0; i < p->count; i++) {
            iVec2 pos = ivec2_plus(bps[j], p->coords[i]);
            if (!IN_GRID(pos) || grid.colors[pos.y][pos.x] != COLOR_EMPTY) {
                return false;
            }
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

static PatternVector grid_sweep() {
    for (i32 i = 0; i < GRID_HEIGHT; i++)
        for (i32 j = 0; j < GRID_WIDTH; j++)
            grid.visited[i][j] = false;

    PatternVector matched_patterns = VECTOR_INIT();
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
                pattvec_add(&matched_patterns, *p);
                pattern_normalize(p);
            }
        }
    }

    return matched_patterns;
}

static void enter_falling_state()
{
    if (!pattbuf_dequeue(&pattern_buffer, &cur_piece.patt)) {
        pattern_generate(&cur_piece.patt);
    }
    cur_piece.pos = vec2(3 * 16.f, 0);
    rotation.in = false;

    if (!is_valid_pattern_pos(cur_piece.pos, &cur_piece.patt)) {
        cur_state = STATE_END;
    } else {
        cur_state = STATE_FALLING;
    }
    state_timer = 0;
}

static bool is_pos_y_less(FallingList *p, void *pos)
{
    return p->elem.pos.y < ((Vector2 *)pos)->y;
}

void add_falling_block(Vector2 pos, GridColor color)
{
    FallingList **p = falling_list_findf(&falling_blocks, is_pos_y_less, &pos);
    FallingBlock block = {
        .color = color,
        .pos = pos,
        .vel = 16.0f,
    };
    falling_list_add(p, block);
}

void game_load()
{
    field_ui = load_texture("resources/ui.png");
    field_ui_bg = load_texture("resources/ui_bg.png");
    preview1 = load_texture("resources/ui_preview1.png");
    preview2 = load_texture("resources/ui_preview2.png");
    blocks = load_texture("resources/blocks.png");
    volume_img = load_texture("resources/volume.png");

    match_sfx = load_sound("resources/match.wav");
    rotate_sfx = load_sound("resources/rotate.wav");
}

void game_unload()
{
    UnloadTexture(field_ui);
    UnloadTexture(field_ui_bg);
    UnloadTexture(preview1);
    UnloadTexture(preview2);
    UnloadTexture(blocks);
    UnloadTexture(volume_img);

    UnloadSound(match_sfx);
    UnloadSound(rotate_sfx);
}

void game_enter() {
    for (i32 y = 0; y < GRID_HEIGHT; y++)
        for (i32 x = 0; x < GRID_WIDTH; x++)
            grid.colors[y][x] = COLOR_EMPTY;

    pattbuf_init(&pattern_buffer);
    // pattbuf_init(&matched_patterns);

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

bool is_key_down(KeyboardKey key, i32 timer, i32 time)
{
    int k = raylib_key_to_input_key(key);
    if (timer % time == 0) {
        bool down = input_buffers[k];
        input_buffers[k] = false;
        return down;
    }
    return false;
}

static bool is_hovering(i32 x, i32 base_y)
{
    for (i32 y = base_y+1; y < GRID_HEIGHT; y++) {
        if (grid.colors[y][x] == COLOR_EMPTY) {
            return true;
        }
    }
    return false;
}

void falling_state_update(i32 frame)
{
    if (block_down && !IsKeyDown(KEY_DOWN)) {
        block_down = false;
    }

    if (rotation.in) {
        if (state_timer - rotation.init_timer >= 8) {
            memcpy(&cur_piece.patt, &rotation.pattern, sizeof(Pattern));
            rotation.in = false;
        }
    } else if (IsKeyPressed(KEY_Z) != IsKeyPressed(KEY_X)) {
        PlaySound(rotate_sfx);
        memcpy(&rotation.pattern, &cur_piece.patt, sizeof(Pattern));
        pattern_rotate(&rotation.pattern, IsKeyPressed(KEY_Z));
        if (is_valid_pattern_pos(cur_piece.pos, &rotation.pattern)) {
            rotation.in = true;
            rotation.init_timer = state_timer;
        }
    }

    if (IsKeyDown(KEY_LEFT)) {
        input_buffers[INPUT_LEFT] = true;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        input_buffers[INPUT_RIGHT] = true;
    }
    if (IsKeyDown(KEY_DOWN)) {
        input_buffers[INPUT_DOWN] = true;
    }

    i32 xdir = (-is_key_down(KEY_LEFT, state_timer, 4) + is_key_down(KEY_RIGHT, state_timer, 4)) * 16;
    if (!is_valid_pattern_pos(Vector2Add(cur_piece.pos, vec2(xdir, 0)), &cur_piece.patt)) {
        xdir = 0;
    }
    i32 ydir = (state_timer % 16 == 0) * 4 + (!block_down && IsKeyDown(KEY_DOWN)) * 6;
    cur_piece.pos = Vector2Add(cur_piece.pos, vec2(xdir, ydir));
    if (!is_valid_pattern_pos(cur_piece.pos, &cur_piece.patt)) {
        cur_piece.pos.y = floorf(cur_piece.pos.y / 16.f) * 16.f;
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
             Vector2 pos = Vector2Add(cur_piece.pos, Vector2Scale(as_vec2(cur_piece.patt.coords[i]), 16));
             add_falling_block(pos, cur_piece.patt.color[i]);
        }
        cur_piece.pos = vec2(-100, -100);
        rotation.in = false;
        cur_state = STATE_SETTLING;
        state_timer = 0;
        return;
    }

    if (frame % 64 == 0) {
        volume_cooldown = CLAMP(volume_cooldown + 1, 0, COOLDOWN_MAX);
        if (volume_cooldown == COOLDOWN_MAX) {
            rotation.in = false;
            cur_state = STATE_END;
        }
    }
}

void game_update(f32 dt, i32 frame) {
    state_timer++;

    i32 local_score = 0;
    i32 matched_count = 0;

    switch (cur_state) {
    case STATE_FALLING:
        falling_state_update(frame);
        break;
    case STATE_SETTLING:
        block_down = true;

        bool all_settled = true;
        for (FallingList **p = &falling_blocks; *p; ) {
            FallingBlock *b = &(*p)->elem;
            b->vel += 1000.0f * dt;
            b->pos.y += b->vel * dt;
            iVec2 pos_up = IVEC2(b->pos.x / 16.f, b->pos.y / 16.f);
            iVec2 pos_dn = IVEC2(b->pos.x / 16.f, (b->pos.y + 15.f) / 16.f);
            if (pos_dn.y == GRID_HEIGHT || grid.colors[pos_dn.y][pos_dn.x] != COLOR_EMPTY) {
                // add falling block to grid and remove it from the list
                grid.colors[pos_up.y][pos_up.x] = b->color;
                falling_list_remove(p);
            } else {
                all_settled = false;
                p = &(*p)->next;
            }
        }

        if (all_settled) {
            PatternVector matched_patterns = grid_sweep();
            if (matched_patterns.size > 0) {
                for (size_t i = 0; i < matched_patterns.size; i++) {
                    Pattern out = matched_patterns.data[i];
                    PlaySound(match_sfx);
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
                score += local_score * matched_count;
                total_matched += matched_count;

                for (i32 y = 0; y < GRID_HEIGHT; y++) {
                    for (i32 x = 0; x < GRID_WIDTH; x++) {
                        if (grid.colors[y][x] != COLOR_EMPTY && is_hovering(x, y)) {
                            add_falling_block(vec2(x * 16, y * 16), grid.colors[y][x]);
                            grid.colors[y][x] = COLOR_EMPTY;
                        }
                    }
                }
            } else {
                enter_falling_state();
            }
            pattvec_free(&matched_patterns);
        }
        break;

    default:
        break;
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
    if (pattbuf_size(&pattern_buffer) <= n) {
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

const i32 block_frames[] = { 0, 1, 2, 3, 2, 1, };

void game_draw(f32 dt, i32 frame) {
    Vector2 grid_pos = vec2(96, 16);

    // background
    i32 bg_frame = ((frame / 8) % 2) * 128;
    DrawTextureRec(field_ui_bg, rec(vec2(bg_frame, 0), vec2(128, 208)), grid_pos, WHITE);

    // previews
    draw_preview(0, vec2(7,  28), vec2(68, 47), preview1, frame);
    draw_preview(1, vec2(8, 165), vec2(64, 65), preview2, frame);

    // current piece and grid
    i32 block_frameno = block_frames[(frame/64) % COUNT_OF(block_frames)];

    if (!rotation.in) {
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
            Vector2 pos = Vector2Add(cur_piece.pos, Vector2Scale(as_vec2(cur_piece.patt.coords[i]), GRID_CELL_SIDE));
            draw_block(Vector2Add(pos, grid_pos), cur_piece.patt.color[i], block_frameno);
        }
    } else {
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
            Vector2 from_pos = Vector2Scale(as_vec2(cur_piece.patt.coords[i]), GRID_CELL_SIDE);
            Vector2 to_pos   = Vector2Scale(as_vec2(rotation.pattern.coords[i]), GRID_CELL_SIDE);
            float t = (state_timer - rotation.init_timer) / 8.f;
            Vector2 pos = Vector2Add(cur_piece.pos, Vector2Lerp(from_pos, to_pos, t));
            draw_block(Vector2Add(pos, grid_pos), cur_piece.patt.color[i], block_frameno);
        }
    }

    for (i32 y = 0; y < GRID_HEIGHT; y++) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            if (grid.colors[y][x] != COLOR_EMPTY) {
                Vector2 pos = Vector2Add(Vector2Scale(vec2(x, y), GRID_CELL_SIDE), GRID_POS);
                draw_block(pos, grid.colors[y][x], block_frameno);
            }
        }
    }

    for (FallingList *p = falling_blocks; p; p = p->next) {
        draw_block(Vector2Add(grid_pos, p->elem.pos), p->elem.color, block_frameno);
    }

    apool_update(dt);

    // foreground
    DrawTexture(field_ui, 0, 0, WHITE);

    // score
    draw_text_centered(TextFormat("%d", score),         vec2(280, 34), 2, WHITE, GetFontDefault(), 12, 1);
    draw_text_centered(TextFormat("%d", total_matched), vec2(280, 54), 2, WHITE, GetFontDefault(), 12, 1);

    // volume
    Vector2 volume_size = vec2(66 - volume_cooldown, 45 - volume_cooldown);
    DrawTexturePro(
        volume_img,
        rec(vec2(0, 0), volume_size),
        rec(vec2(247 + volume_cooldown, 174 + volume_cooldown), volume_size),
        vec2(0, 0), 0, WHITE
    );
}

GameScreen game_exit()
{
    if (cur_state == STATE_END) {
        return SCREEN_TITLE;
    }
    return SCREEN_UNKNOWN;
}
