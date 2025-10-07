#pragma once

#include "raylib.h"
#include "stdbool.h"

void sound_set_enabled(bool enabled);
bool sound_enabled();
void sound_play(Sound sound);