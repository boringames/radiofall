#include "hiscore.h"

#include "util.h"

static i64 hiscore = 0;
void hiscore_init() {
    hiscore_load();
}

void hiscore_load() {
    hiscore = data_read_i64("hiscore");
    if (hiscore == -1) {
        hiscore_set(0);
    }
}

void hiscore_save() {
    if (!data_write_i64("hiscore", hiscore)) {
        GAME_LOG_ERR("failed saving hiscore value: %d", hiscore);
    }
}

void hiscore_set(i64 value) {
    hiscore = value;
    hiscore_save();
}

i64 hiscore_get() {
    return hiscore;
}

void hiscore_try_set(i64 value) {
    if (value > hiscore) {
        hiscore_set(value);
    }
}