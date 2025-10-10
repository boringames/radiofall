#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include "data.h"
#include "os.h"

static const char *radiofall_dir() {
    const char *home = home_dir();
    if (!home) {
        return NULL;
    }
    return TextFormat("%s/.radiofall", home);
}

bool data_init() {
    const char *radiofall_data_dir = radiofall_dir();
    if (!DirectoryExists(radiofall_data_dir)) {
        MakeDirectory(radiofall_data_dir);
    }
    return true;
}

i64 data_read_i64(const char *key) {
    const char *radiofall_data_dir = radiofall_dir();
    i32 sz = 0; u8 *data = LoadFileData(TextFormat("%s/%s", radiofall_data_dir, key), &sz);
    if (sz != sizeof(i64)) {
        RL_FREE(data);
        return -1;
    }

    i64 value = 0;
    memcpy(&value, data, sizeof(i64));
    return value;
}

bool data_write_i64(const char *key, i64 value) {
    const char *radiofall_data_dir = radiofall_dir();
    bool ok = SaveFileData(TextFormat("%s/%s", radiofall_data_dir, key), &value, sizeof(i64));
    return ok;
}
