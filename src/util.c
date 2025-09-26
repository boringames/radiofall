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

