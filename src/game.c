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
#include "hiscore.h"
#include "sound.h"

#define PATTERN_MATCH_MIN 4

// grid stuff
struct {
    GridColor colors[GRID_HEIGHT][GRID_WIDTH];
} grid;

static const Vector2 GRID_POS = {96, 16};

#define IN_GRID(v) (v.y >= 0 && v.y < GRID_HEIGHT && v.x >= 0 && v.x < GRID_WIDTH)

// contains matched patterns, queued up to be placed at the top
PatternBuffer pattern_buffer;

// when a new pattern is needed, it is copied here
struct {
    Vector2 pos;
    Pattern patt;
    // when the player inputs a rotation, it takes a few frames to complete
    // meanwhile, its info is stored here
    struct {
        Pattern pattern;
        bool playing;
        i32 init_timer;
    } rotation;
    bool falling;
} cur_piece;

i32 score = 0;
i32 total_matched = 0;

// falling blocks, generated when a line is matched
typedef struct {
    GridColor color;
    Vector2 pos;
    float vel;
} FallingBlock;

LLIST_DECLARE(FallingList, FallingBlock, falling_list)
LLIST_DEFINE(FallingList, FallingBlock, falling_list)

FallingList *falling_blocks;

typedef struct {
    iVec2 pos; // origin of the pattern match and just "sweeped"
    i32 pcount;
} MatchInfo;

MatchInfo *matchdup(MatchInfo *m)
{
    MatchInfo *n = MemAlloc(sizeof(MatchInfo));
    n->pos = m->pos;
    n->pcount = m->pcount;
    return n;
}

enum {
    STATE_RUNNING,
    STATE_GAMEOVER,
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

#define COOLDOWN_MAX 20

// volume indicator. counts up, when it gets to COOLDOWN_MAX the game is over
i32 volume_cooldown = 0;

// key input buffers
// we check input for these keys each frame, but their action
// is delayed by a few frames so that they're slowed down
#define INPUT_LEFT 0
#define INPUT_RIGHT 1

bool input_buffers[2] = {0};

int raylib_key_to_input_key(KeyboardKey key)
{
    return key == KEY_LEFT  ? INPUT_LEFT
         : key == KEY_RIGHT ? INPUT_RIGHT
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

static void find_pattern(iVec2 pos, GridColor color, Pattern *pattern, bool visited[GRID_HEIGHT][GRID_WIDTH]) {
    visited[pos.y][pos.x] = true;
    pattern->coords[pattern->count] = pos;
    pattern->count++;
    for (i32 i = 0; i < 4; i++) {
        iVec2 neighbor = ivec2_plus(pos, DIRS[i]);
        if (!visited[neighbor.y][neighbor.x] && IN_GRID(neighbor) && grid.colors[neighbor.y][neighbor.x] == color) {
            find_pattern(neighbor, color, pattern, visited);
        }
    }
}

static PatternVector grid_sweep() {
    bool visited[GRID_HEIGHT][GRID_WIDTH] = {0};
    PatternVector matched_patterns = VECTOR_INIT();

    for (i32 i = 0; i < GRID_HEIGHT; i++) {
        for (i32 j = 0; j < GRID_WIDTH; j++) {
            if (grid.colors[i][j] == COLOR_EMPTY || visited[i][j]) {
                continue;
            }

            Pattern *p = pattbuf_enqueue_ptr(&pattern_buffer);
            if (!p) {
                continue;
            }

            p->count = 0;
            find_pattern(IVEC2(j, i), grid.colors[i][j], p, visited);
            if (p->count < PATTERN_MATCH_MIN) {
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

static void init_cur_piece()
{
    if (!pattbuf_dequeue(&pattern_buffer, &cur_piece.patt)) {
        pattern_generate(&cur_piece.patt);
    }
    i32 x = pattern_max(&cur_piece.patt).x + 1;
    cur_piece.pos = vec2((GRID_WIDTH - x)/2 * 16.f, 0);
    cur_piece.rotation.playing = false;

    if (!is_valid_pattern_pos(cur_piece.pos, &cur_piece.patt)) {
        cur_state = STATE_GAMEOVER;
    }

    cur_piece.falling = true;

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

    // hiscore state
    hiscore_load();
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

    score = 0;
    volume_cooldown = 0;
    total_matched = 0;
    state_timer = 0;
    init_cur_piece();
}

bool animate_score(void *context, f32 dt, i32 frameno) {
    MatchInfo *m = (MatchInfo *)(context);
    DrawText(TextFormat("+%d", m->pcount),
            (m->pos.x * GRID_CELL_SIDE) + GRID_POS.x + cos(frameno) ,
            (m->pos.y * GRID_CELL_SIDE) + GRID_POS.y + sin(frameno),
            12, WHITE);
    if (frameno == 128) {
        free(m);
        return true;
    }
    return false;
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

void falling_piece_update(i32 frame)
{
    if (!cur_piece.falling) {
        return;
    }

    if (block_down && !IsKeyDown(KEY_DOWN)) {
        block_down = false;
    }

    if (cur_piece.rotation.playing) {
        if (state_timer - cur_piece.rotation.init_timer >= 8) {
            memcpy(&cur_piece.patt, &cur_piece.rotation.pattern, sizeof(Pattern));
            cur_piece.rotation.playing = false;
        }
    } else if (IsKeyPressed(KEY_Z) != IsKeyPressed(KEY_X)) {
        sound_play(rotate_sfx);
        memcpy(&cur_piece.rotation.pattern, &cur_piece.patt, sizeof(Pattern));
        pattern_rotate(&cur_piece.rotation.pattern, IsKeyPressed(KEY_Z));
        if (is_valid_pattern_pos(cur_piece.pos, &cur_piece.rotation.pattern)) {
            cur_piece.rotation.playing = true;
            cur_piece.rotation.init_timer = state_timer;
        }
    }

    if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT)) {
        input_buffers[INPUT_LEFT] = true;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT)) {
        input_buffers[INPUT_RIGHT] = true;
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
        cur_piece.rotation.playing = false;
        cur_piece.falling = false;
        block_down = true;
        state_timer = 0;
        return;
    }
}

void volume_update(i32 frame)
{
    if (cur_piece.falling && frame % 64 == 0) {
        volume_cooldown = CLAMP(volume_cooldown + 1, 0, COOLDOWN_MAX);
        if (volume_cooldown == COOLDOWN_MAX) {
            cur_piece.rotation.playing = false;
            cur_state = STATE_GAMEOVER;
        }
    }
}

void falling_blocks_update(f32 dt, i32 frame)
{
    if (!falling_blocks) {
        return;
    }

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
            p = &(*p)->next;
        }
    }

    if (!falling_blocks) {
        PatternVector matched_patterns = grid_sweep();
        if (matched_patterns.size > 0) {
            i32 matched_count = 0;
            i32 local_score = 0;
            for (ptrdiff_t i = 0; i < matched_patterns.size; i++) {
                Pattern out = matched_patterns.data[i];
                sound_play(match_sfx);
                matched_count++;
                local_score += out.count;
                volume_cooldown = CLAMP(volume_cooldown - out.count, 0, COOLDOWN_MAX);
                iVec2 min = pattern_min(&out);
                pattern_normalize(&out);
                apool_add((Animation) {
                    .anim_update = animate_score,
                    .cur_frame = 0,
                    .data = matchdup(&(MatchInfo) {
                        .pcount = out.count,
                        .pos = ivec2_plus(min, pattern_origin(&out)),
                    })
                });
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

            if (!falling_blocks) {
                init_cur_piece();
            }
        } else {
            init_cur_piece();
        }
        pattvec_free(&matched_patterns);
    }
}

void game_update(f32 dt, i32 frame) {
    state_timer++;

    switch (cur_state) {
    case STATE_RUNNING:
        falling_piece_update(frame);
        volume_update(frame);
        falling_blocks_update(dt, frame);
        break;
    case STATE_GAMEOVER:
        if (IsKeyDown(KEY_ENTER) || IsKeyDown(KEY_Z)) {
            hiscore_try_set(score);
            game_enter();
        } else if (IsKeyDown(KEY_X)) {
            hiscore_try_set(score);
            cur_state = STATE_END;
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
    // background
    i32 bg_frame = ((frame / 8) % 2) * 128;
    DrawTextureRec(field_ui_bg, rec(vec2(bg_frame, 0), vec2(128, 208)), GRID_POS, WHITE);

    // previews
    draw_preview(0, vec2(7,  28), vec2(68, 47), preview1, frame);
    draw_preview(1, vec2(8, 165), vec2(64, 65), preview2, frame);

    // current piece
    i32 block_frameno = block_frames[(frame/64) % COUNT_OF(block_frames)];

    if (!cur_piece.rotation.playing) {
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
            Vector2 pos = Vector2Add(cur_piece.pos, Vector2Scale(as_vec2(cur_piece.patt.coords[i]), GRID_CELL_SIDE));
            draw_block(Vector2Add(pos, GRID_POS), cur_piece.patt.color[i], block_frameno);
        }
    } else {
        for (i32 i = 0; i < cur_piece.patt.count; i++) {
            Vector2 from_pos = Vector2Scale(as_vec2(cur_piece.patt.coords[i]), GRID_CELL_SIDE);
            Vector2 to_pos   = Vector2Scale(as_vec2(cur_piece.rotation.pattern.coords[i]), GRID_CELL_SIDE);
            float t = (state_timer - cur_piece.rotation.init_timer) / 8.f;
            Vector2 pos = Vector2Add(cur_piece.pos, Vector2Lerp(from_pos, to_pos, t));
            draw_block(Vector2Add(pos, GRID_POS), cur_piece.patt.color[i], block_frameno);
        }
    }

    // grid
    for (i32 y = 0; y < GRID_HEIGHT; y++) {
        for (i32 x = 0; x < GRID_WIDTH; x++) {
            if (grid.colors[y][x] != COLOR_EMPTY) {
                Vector2 pos = Vector2Add(Vector2Scale(vec2(x, y), GRID_CELL_SIDE), GRID_POS);
                draw_block(pos, grid.colors[y][x], block_frameno);
            }
        }
    }

    for (FallingList *p = falling_blocks; p; p = p->next) {
        draw_block(Vector2Add(GRID_POS, p->elem.pos), p->elem.color, block_frameno);
    }

    apool_update(dt);

    // foreground
    DrawTexture(field_ui, 0, 0, WHITE);

    // score
    draw_text_centered(TextFormat("%d", score),         vec2(280, 34), 2, WHITE, GetFontDefault(), 12, 1);
    draw_text_centered(TextFormat("%d", total_matched), vec2(280, 54), 2, WHITE, GetFontDefault(), 12, 1);
    draw_text_centered(TextFormat("%d", hiscore_get()), vec2(280, 74), 2,       YELLOW, GetFontDefault(), 12, 1);

    // volume
    Vector2 volume_size = vec2(66 - volume_cooldown, 45 - volume_cooldown);
    DrawTexturePro(
        volume_img,
        rec(vec2(0, 0), volume_size),
        rec(vec2(247 + volume_cooldown, 174 + volume_cooldown), volume_size),
        vec2(0, 0), 0, WHITE
    );

    if (cur_state == STATE_GAMEOVER) {
        draw_text_centered("GAME OVER",       vec2((f32)(RESOLUTION[0]/2), (f32)(RESOLUTION[1]/4)),      0, WHITE, GetFontDefault(), 16, 1);
        draw_text_centered("[Z] or [ENTER] restart", vec2((f32)(RESOLUTION[0]/2), (f32)(RESOLUTION[1]/4 + 20)), 0, WHITE, GetFontDefault(), 12,  1);
        draw_text_centered("[X] menu",      vec2((f32)(RESOLUTION[0]/2), (f32)(RESOLUTION[1]/4 + 40)), 0, WHITE, GetFontDefault(), 12,  1);
    }
}

GameScreen game_exit()
{
    if (cur_state == STATE_END) {
        return SCREEN_TITLE;
    }
    return SCREEN_UNKNOWN;
}
