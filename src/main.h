#pragma once

#include "util.h"

typedef enum GameScreen {
    SCREEN_TITLE,
    SCREEN_GAMEPLAY,
    SCREEN_UNKNOWN,
    SCREEN_QUIT,
} GameScreen;

static const i32 RESOLUTION[] = { 320, 240 };
