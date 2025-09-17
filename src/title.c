#include <raylib.h>
#include "screens.h"
#include "const.h"

bool start = false;

void title_init()
{
    start = false;
}

void title_update(f32 dt, i32 frame)
{
    if (IsKeyDown(KEY_ENTER)) {
        start = true;
    }
}

void title_draw()
{
    const char *text = "title... (press RETURN)";
    int width = MeasureText(text, 16);
    DrawText(text, RESOLUTION[0]/2 - width/2, RESOLUTION[1]/2, 16, WHITE);
}

void title_unload()
{

}

int title_finish()
{
    return start ? SCREEN_GAMEPLAY : SCREEN_UNKNOWN;
}
