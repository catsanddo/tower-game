#ifndef CONFIG_H
#define CONFIG_H

#define SCALE 3
#define WIDTH 160
#define HEIGHT 144
#define SCREEN_WIDTH (WIDTH * SCALE)
#define SCREEN_HEIGHT (HEIGHT * SCALE)

#define TILE 16

#define FONT_SIZE 7
#define FONT_PATH "res/fonts/PublicPixel.ttf"
#define FONT_COLOR (SDL_Color) { 245, 245, 245, 255 }
#define FONT_BG (SDL_Color) { 0, 0, 0, 255 }
#define TBOX_TOP (SDL_Rect) { 10, 10, 140, 38 }
#define TBOX_BOTTOM (SDL_Rect) { 10, 100, 140, 38 }
#define TBOX_LIMIT 64
#define TBOX_ROWS 5

#define ENTITY_SPEED 1
#define DOWN_ANIMATION 0
#define RIGHT_ANIMATION 1
#define LEFT_ANIMATION 2
#define UP_ANIMATION 3

#define MAX_FPS 60
#define MAX_ENTITIES 25
#define FLAG_LIMIT 1024

#define INITIAL_MAP "res/maps/Forest.json"

#endif
