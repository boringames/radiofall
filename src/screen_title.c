#include <raylib.h>
#include "screens.h"

bool start = false;

void title_init()
{
    start = false;
}

void title_update()
{
    if (IsKeyDown(KEY_ENTER)) {
        start = true;
    }
}

void title_draw()
{

}

void title_unload()
{

}

int title_finish()
{
    return start ? SCREEN_GAMEPLAY : SCREEN_UNKNOWN;
}
