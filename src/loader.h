#pragma once

#include "core/types.h"

typedef i32 LoaderMap[GRID_HEIGHT][GRID_WIDTH];

LoaderMap *loader_load_maps(const char *filepath, i32 *n_maps);
