#pragma once

#include "util.h"

void hiscore_load();
void hiscore_save();
void hiscore_set(i64 value);
i64 hiscore_get();
void hiscore_try_set(i64 value);