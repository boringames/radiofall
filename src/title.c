#include <raylib.h>
#include "screens.h"

bool start = false;

void title_init()
{
    start = false;
}

void title_update(f32 dt)
{
    if (IsKeyDown(KEY_ENTER)) {
        start = true;
    }
}

void title_draw()
{
    int resolution[] = {
        GetRenderWidth(),
        GetRenderHeight(),
    };
    const char *text = "title... (press RETURN)";
    int width = MeasureText(text, 32);
    DrawText(text, resolution[0]/2 - width/2, resolution[1]/2, 32, WHITE);
}

void title_unload()
{

}

int title_finish()
{
    return start ? SCREEN_GAMEPLAY : SCREEN_UNKNOWN;
}
