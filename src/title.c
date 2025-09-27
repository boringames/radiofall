#include <raylib.h>
#include "title.h"
#include "util.h"

bool start = false;

enum {
    MENU_PLAY,
    MENU_SCORES,
    MENU_OPTIONS,
    MENU_EXIT,
    _MENU_COUNT
};

Texture2D title_ui;
Texture2D title_ui_bg;

Sound menu_select_sfx;
Sound menu_scroll_sfx;

i32 cur_menu_item = MENU_PLAY;

void title_load()
{
    title_ui = load_texture("resources/title_screen.png");
    title_ui_bg = load_texture("resources/title_screen_bg.png");

    menu_select_sfx = load_sound("resources/match.wav");
    menu_scroll_sfx = load_sound("resources/rotate.wav");
}

void title_unload()
{

}

void title_enter()
{
    start = false;
}

void title_update(f32 dt, i32 frame)
{
    if (IsKeyDown(KEY_ENTER)) {
        start = true;
    }

    bool is_right = (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && !(frame % 4);
    bool is_left = (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && !(frame % 4);
    if (is_right != is_left) {
        PlaySound(menu_scroll_sfx);
    }

    if (is_right) {
        cur_menu_item = (cur_menu_item + 1) % _MENU_COUNT;
    } else if (is_left) {
        cur_menu_item = cur_menu_item - 1 < 0 ? _MENU_COUNT - 1 : cur_menu_item - 1;
    }
}

float yoff = 0.0f;
void title_draw(f32 dt, i32 frameno)
{
    DrawTexture(title_ui_bg, 0, 0, WHITE);
    DrawTexture(title_ui, 0, 0, WHITE);
}

GameScreen title_exit()
{
    return start ? SCREEN_GAMEPLAY : SCREEN_UNKNOWN;
}
