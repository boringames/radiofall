#include <raylib.h>
#include <math.h>
#include "title.h"
#include "util.h"
#include "animation_pool.h"

#define COLOR_UNSELECTED ((Color){200, 200, 200, 255})
#define COLOR_SELECTED (WHITE)

bool start = false;

enum {
    MENU_PLAY,
    MENU_NEXT,
    MENU_MUTE,
    MENU_EXIT,
    _MENU_COUNT
};

Texture2D title_ui;
Texture2D title_ui_bg;
Texture2D play_button;
Texture2D next_button;
Texture2D mute_button;
Texture2D exit_button;
Texture2D tapes;

Sound menu_select_sfx;
Sound menu_scroll_sfx;

i32 cur_menu_item = MENU_PLAY;

void title_load()
{
    title_ui = load_texture("resources/title_screen.png");
    title_ui_bg = load_texture("resources/title_screen_bg.png");
    tapes = load_texture("resources/title_tapes.png");
    play_button = load_texture("resources/play_button.png");
    next_button = load_texture("resources/next_button.png");
    mute_button = load_texture("resources/mute_button.png");
    exit_button = load_texture("resources/exit_button.png");

    menu_select_sfx = load_sound("resources/stereo_button.wav");
    menu_scroll_sfx = load_sound("resources/rotate.wav");
}

void title_unload()
{

}

void title_enter()
{
    start = false;
}

f32 _press_cd = 0.0f;
void title_update(f32 dt, i32 frame)
{
    if (IsKeyDown(KEY_ENTER) && cur_menu_item == MENU_PLAY) {
        SetSoundVolume(menu_select_sfx, 0.2f);
        PlaySound(menu_select_sfx);
        start = true;
    };

    if (_press_cd > 0.1f) {
        _press_cd = 0;
        bool is_right = (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT));
        bool is_left = (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT));
        if (is_right != is_left)
            PlaySound(menu_scroll_sfx);

        if (is_right)
            cur_menu_item = (cur_menu_item + 1) % _MENU_COUNT;
        else if (is_left)
            cur_menu_item = cur_menu_item - 1 < 0 ? _MENU_COUNT - 1 : cur_menu_item - 1;
    }
    _press_cd += dt;
}

float tapes_rot = 1.0;
float yoff = 0.0f;
void title_draw(f32 dt, i32 frameno)
{
    DrawTexture(title_ui_bg, 0, 0, WHITE);
    {
        f32 floating_off = (sin(GetTime() * 10)) + 1;
        if (start) {
            floating_off *= 3;
        }

        apool_update(dt);

        DrawTexture(play_button,
            64 + play_button.width * 0,
            104 + (cur_menu_item == MENU_PLAY ? floating_off : 0),
            cur_menu_item == MENU_PLAY ? COLOR_SELECTED : COLOR_UNSELECTED);

        DrawTexture(next_button,
            64 + play_button.width * 1,
            104 + (cur_menu_item == MENU_NEXT ? floating_off : 0),
            cur_menu_item == MENU_NEXT ? COLOR_SELECTED : COLOR_UNSELECTED);

        DrawTexture(mute_button,
            64 + play_button.width * 2,
            104 + (cur_menu_item == MENU_MUTE ? floating_off : 0),
            cur_menu_item == MENU_MUTE ? COLOR_SELECTED : COLOR_UNSELECTED);

        DrawTexture(exit_button,
            64 + play_button.width * 3,
            104 + (cur_menu_item == MENU_EXIT ? floating_off : 0),
            cur_menu_item == MENU_EXIT ? COLOR_SELECTED : COLOR_UNSELECTED);
    }
    Rectangle src = {0.0f, 0.0f, tapes.width * tapes_rot, tapes.height};
    Rectangle dst = {64.0f, 144.0f, tapes.width, tapes.height};
    if (frameno % 8 == 0) {
        tapes_rot *= -1;
    }

    DrawTexturePro(tapes, src, dst, (Vector2){0, 0}, 0, WHITE);

    DrawTexture(title_ui, 0, 0, WHITE);
    DrawRectangle(40 + ((sin(GetTime()) + 1.0)/2.0) * 240, 130, 2, 6, LIGHTGRAY);
}

GameScreen title_exit()
{
    if (start) {
        return SCREEN_GAMEPLAY;
    }

    return SCREEN_UNKNOWN;
}
