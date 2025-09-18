#ifndef LOADER_H
#define LOADER_H

#include "const.h"
#include "core/types.h"

typedef i32 LoaderMap[GRID_HEIGHT][GRID_WIDTH];

LoaderMap *loader_load_maps(const char *filepath, i32 *n_maps);

#endif // LOADER_H
