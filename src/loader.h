#ifndef LOADER_H
#define LOADER_H

#include "const.h"
#include "core/types.h"

i32 (*loader_load_maps(const char *filepath, i32 *n_maps))[GRID_HEIGHT][GRID_WIDTH];

#endif // LOADER_H
