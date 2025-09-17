#include "loader.h"
#include "const.h"
#include "core/log.h"

#include <raylib.h>

#define LOADER_MAX_MAPS 20

i32 _maps[LOADER_MAX_MAPS][GRID_HEIGHT][GRID_WIDTH];

i32 (*loader_load_maps(const char *filepath, i32 *n_maps))[GRID_HEIGHT][GRID_WIDTH] {
    i32 size = 0; *n_maps = 0;
    const char *buf = (const char *)LoadFileData(filepath, &size);
    {
        i32 m = 0; i32 cursor = 0;
        for (i32 k=0; k<size && m<LOADER_MAX_MAPS; k++) {
            if (buf[k] == ' ' || buf[k] == '\n' || (k%(GRID_WIDTH + 1) == 0)) continue;
            else cursor++;
            i32 i = (cursor/GRID_WIDTH)%GRID_HEIGHT; i32 j = cursor%GRID_WIDTH;
            if (cursor >= GRID_WIDTH && i%GRID_HEIGHT==0) m++;
            if (buf[k]< '0' || buf[k] > '9')
                return _maps;
            _maps[m][i][j] = buf[k] - '0';
        }
        *n_maps = m;
    }
    MemFree((u8 *)buf);
    return _maps;
}
