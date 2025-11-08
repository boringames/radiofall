#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include "title.h"
#include "util.h"
#include "animation_pool.h"
#include "sound.h"
#include "main.h"

#define COLOR_UNSELECTED ((Color){200, 200, 200, 255})
#define COLOR_SELECTED (WHITE)

#define TITLE_TEXT_COLOR ((Color) { 0xff, 0xd4, 0x44, 0xff })
#define TITLE_TEXT_COLOR_OUTLINE ((Color) { 0x7d, 0x68, 0x22, 0xff })

bool start = false;
bool quit = false;

enum {
    MENU_PLAY,
    MENU_NEXT,
    MENU_MUTE,
    MENU_QUIT,
    _MENU_COUNT
};

Texture2D title_ui;
Texture2D title_ui_bg;
Texture2D title_ui_bg_blocks;
Texture2D play_button;
Texture2D next_button;
Texture2D mute_button;
Texture2D exit_button;
Texture2D tapes;

Sound menu_select_sfx;
Sound menu_scroll_sfx;

Sound menu_track;

Shader text_shader;
Font *text_font;

i32 cur_menu_item = MENU_PLAY;

void title_load()
{
    title_ui = load_texture("resources/title-radio.png");
    title_ui_bg = load_texture("resources/title-bg.png");
    title_ui_bg_blocks = load_texture("resources/title-bg-blocks.png");
    tapes = load_texture("resources/title_tapes.png");
    play_button = load_texture("resources/play_button.png");
    next_button = load_texture("resources/next_button.png");
    mute_button = load_texture("resources/mute_button.png");
    exit_button = load_texture("resources/exit_button.png");

    menu_select_sfx = load_sound("resources/stereo_button.wav");
    menu_scroll_sfx = load_sound("resources/rotate.wav");
    menu_track = load_sound("resources/give_her_shadow.wav");
    SetSoundPitch(menu_track, 1.15f);

    text_shader = load_shader(NULL, "resources/text-outline-sdf.glsl");
    int codepoints[90-65];
    for (int i = 65; i <= 90; i++) {
        codepoints[i-65] = i;
    }
    text_font   = load_font_sdf("resources/PollerOne-Regular.ttf", 32, codepoints, COUNT_OF(codepoints));
}

void title_unload()
{

}

void title_enter()
{
    start = false;
}

f32 _enter_press_cd = 0.0f;
f32 _arrow_press_cd = 0.0f;
i32 _prev_menu_item = MENU_QUIT;
void title_update(f32 dt, i32 frame)
{
    if (!sound_playing(menu_track)) {
        sound_play(menu_track);
    }

    if (_prev_menu_item != cur_menu_item) _enter_press_cd = 0;
    if (_enter_press_cd > 0.05f) {
        _enter_press_cd = 0;
        if (IsKeyDown(KEY_ENTER)) {
            if (cur_menu_item == MENU_PLAY) {
                start = true;
            }

            if (cur_menu_item == MENU_MUTE) {
                sound_set_enabled(!sound_enabled());
            }

            if (cur_menu_item == MENU_QUIT) {
                quit = true;
            }

            SetSoundVolume(menu_select_sfx, 0.7f);
            sound_play(menu_select_sfx);
        }
    }

    if (_arrow_press_cd > 0.075f) {
        _arrow_press_cd = 0;

        bool is_right = (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
        bool is_left = (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
        if (is_right != is_left)
            sound_play(menu_scroll_sfx);

        if (is_right)
            cur_menu_item = (cur_menu_item + 1) % _MENU_COUNT;
        else if (is_left)
            cur_menu_item = cur_menu_item - 1 < 0 ? _MENU_COUNT - 1 : cur_menu_item - 1;
    }

    _arrow_press_cd += dt;
    _enter_press_cd += dt;
    _prev_menu_item = cur_menu_item;
}

void title_draw(f32 dt, i32 frameno)
{
    DrawTexturePro(
        title_ui_bg,
        (Rectangle) { .x =  0, .y = 0, .width = title_ui_bg.width, .height = title_ui_bg.height },
        (Rectangle) { .x =  0, .y = 0, .width = title_ui_bg.width, .height = title_ui_bg.height },
        VEC2ZERO, 0, WHITE
    );

    DrawTexturePro(
        title_ui_bg_blocks,
        (Rectangle) { .x = 0, .y = frameno/2, .height = title_ui_bg_blocks.height, .width = title_ui_bg_blocks.width },
        (Rectangle) { .x = 0, .y = 0, .height = 240, .width = 320},
        VEC2ZERO, 0, WHITE
    );

    apool_update(dt);

    {
        f32 floating_off = start ? (sin(GetTime() * 10) * 2) + 4 : (sin(GetTime() * 10)) + 1;

        struct
        {
            Texture texture;
            i32 menu_item;
        } buttons[] = {
            {play_button, MENU_PLAY},
            {next_button, MENU_NEXT},
            {mute_button, MENU_MUTE},
            {exit_button, MENU_QUIT}};

        for (int i = 0; i < 4; i++)
        {
            f32 x = 64 + buttons[i].texture.width * i;
            f32 y = 104 + (cur_menu_item == buttons[i].menu_item ? floating_off : 0);
            Color color = (cur_menu_item == buttons[i].menu_item ? COLOR_SELECTED : COLOR_UNSELECTED);
            DrawTexture(buttons[i].texture, x, y, color);
        }
    }

    DrawTexturePro(
        tapes,
        (Rectangle) {0.0f, 0.0f, tapes.width * ((frameno / 8) % 2 == 0 ? -1 : 1), tapes.height},
        (Rectangle) {64.0f, 144.0f, tapes.width, tapes.height},
        vec2(0, 0), 0, WHITE
    );

    DrawTexture(title_ui, 0, 0, WHITE);
    DrawRectangle(40 + ((sin(GetTime()) + 1.0)/2.0) * 240, 130, 2, 6, LIGHTGRAY);

    Vector2 title_size = MeasureTextEx(*text_font, "RADIOFALL", text_font->baseSize, 0);
    Vector2 title_pos  = vec2((RESOLUTION[0] - title_size.x) / 2, 25);
    DrawTextEx(*text_font, "RADIOFALL", Vector2Add(title_pos, vec2(0, 2)), text_font->baseSize, 0, (Color) { 0, 0, 0, 0xff });
    shader_setv3(text_shader, "outline_color", color_to_vec3(TITLE_TEXT_COLOR_OUTLINE));
    shader_setf(text_shader, "smoothing_param", 30.0);
    shader_setf(text_shader, "outline_width_param", 50.0);
    BeginShaderMode(text_shader);
    DrawTextEx(*text_font, "RADIOFALL", title_pos, text_font->baseSize, 0, TITLE_TEXT_COLOR);
    EndShaderMode();
}

GameScreen title_exit()
{
    if (start) {
        StopSound(menu_track);
        return SCREEN_GAMEPLAY;
    }

    if (quit) {
        return SCREEN_QUIT;
    }

    return SCREEN_UNKNOWN;
}
