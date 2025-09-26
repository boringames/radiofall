#include <raylib.h>
#include "title.h"

bool start = false;

void title_load()
{

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
}

void title_draw(f32 dt, i32 frameno)
{
    const char *text = "title... (press RETURN)";
    int width = MeasureText(text, 16);
    DrawText(text, RESOLUTION[0]/2 - width/2, RESOLUTION[1]/2, 16, WHITE);
}

GameScreen title_exit()
{
    return start ? SCREEN_GAMEPLAY : SCREEN_UNKNOWN;
}
