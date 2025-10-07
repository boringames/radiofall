#include "util.h"

#include <stdlib.h>
#include <string.h>
#include <raylib.h>

char *text_insert(const char *text, const char *insert, int position)
{
    int textLen = TextLength(text);
    int insertLen = TextLength(insert);

    char *result = (char *)malloc(textLen + insertLen + 1);

    memcpy(result, text, position);
    memcpy(result + position, insert, insertLen);
    memcpy(result + position + insertLen, text + position, textLen - position);

    result[textLen + insertLen] = '\0';     // Make sure text string is valid!

    return result;
}

void draw_text_centered(const char *text, Vector2 position, int padding, Color color, Font font, float fontSize, float spacing)
{
    Vector2 textSize = MeasureTextEx(font, text, fontSize, spacing);
    Vector2 drawPos = {
        position.x - (textSize.x + 2 * padding) / 2,
        position.y - (textSize.y + 2 * padding) / 2
    };
    DrawTextEx(font, text, drawPos, fontSize, spacing, color);
}

Texture2D load_texture(const char *path)
{
    const char *appdir = GetApplicationDirectory();
    char *realpath = text_insert(appdir, path, strlen(appdir));
    Texture2D t = LoadTexture(realpath);
    SetTextureFilter(t, TEXTURE_FILTER_POINT);
    RL_FREE(realpath);
    return t;
}

Sound load_sound(const char *path)
{
    const char *appdir = GetApplicationDirectory();
    char *realpath = text_insert(appdir, path, strlen(appdir));
    Sound s = LoadSound(realpath);
    RL_FREE(realpath);
    return s;
}

void mem_copy(void *dst, void *src, u64 size)
{
    u8 *d = (u8*)(dst); const u8 *s = (const u8*)(src);
    u64 i = 0;
    for (; i <= size - sizeof(uint64_t); i += sizeof(uint64_t))
        ((uint64_t*)d)[i/sizeof(uint64_t)] = ((const u64*)s)[i/sizeof(u64)];
    for (; i < size; i++)
        d[i] = s[i];
}

