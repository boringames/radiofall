#pragma once

typedef enum GameScreen {
    SCREEN_TITLE,
    SCREEN_OPTIONS,
    SCREEN_GAMEPLAY,
    SCREEN_ENDING,
    SCREEN_UNKNOWN,
} GameScreen;

void title_init();
void title_update();
void title_draw();
void title_unload();
int title_finish();

void options_init();
void options_update();
void options_draw();
void options_unload();
int options_finish();

void gameplay_init();
void gameplay_update();
void gameplay_draw();
void gameplay_unload();
int gameplay_finish();

void ending_init();
void ending_update();
void ending_draw();
void ending_unload();
int ending_finish();
