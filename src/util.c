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

Shader load_shader(const char *vs, const char *fs)
{
    const char *appdir = GetApplicationDirectory();
    char *_vs = vs ? text_insert(appdir, vs, strlen(appdir)) : NULL;
    char *_fs = fs ? text_insert(appdir, fs, strlen(appdir)) : NULL;
    Shader s = LoadShader(_vs, _fs);
    RL_FREE(_vs);
    RL_FREE(_fs);
    return s;
}

Font *load_font_sdf(const char *path, int baseSize, int *codepoints, int cp_count)
{
    const char *appdir = GetApplicationDirectory();
    const char *realpath = text_insert(appdir, path, strlen(appdir));
    int fileSize = 0;
    unsigned char *fileData = LoadFileData(realpath, &fileSize);

    // SDF font generation from TTF font
    Font *fontSDF = (Font *)malloc(sizeof(Font));
    fontSDF->baseSize = baseSize;
    fontSDF->glyphCount = cp_count;
    fontSDF->glyphs = LoadFontData(fileData, fileSize, baseSize, codepoints, cp_count, FONT_SDF);
    Image atlas = GenImageFontAtlas(fontSDF->glyphs, &fontSDF->recs, cp_count, baseSize, 0, 1);
    fontSDF->texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(fileData);
    SetTextureFilter(fontSDF->texture, TEXTURE_FILTER_BILINEAR);
    return fontSDF;
}