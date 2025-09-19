#pragma once

#include <raylib.h>
#include <stdio.h>
#include "core/types.h"

#define COUNT_OF(a) (sizeof(a)/sizeof(a[0]))

#define GAME_LOG(STREAM, FMT, ...) \
        do{fprintf(STREAM, "[%s:%s:%d]: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);}while(0)
#define GAME_LOG_ERR(FMT, ...) \
        do{fprintf(stderr, "[%s:%s:%d][ERROR]: " FMT "\n", __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__);}while(0)
