#pragma once

#include <stdbool.h>
#include "util.h"

bool data_init();

i64 data_read_i64(const char *key);
bool data_write_i64(const char *key, i64 value);