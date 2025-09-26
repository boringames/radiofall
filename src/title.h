#pragma once

#include "main.h"

void title_load();
void title_unload();
void title_enter();
void title_update(f32 dt, i32 frame);
void title_draw(f32 dt, i32 frameno);
GameScreen title_exit();
