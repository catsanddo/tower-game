#ifndef RENDER_H
#define RENDER_H

#include "common.h"
#include "config.h"

#define TILE_FLIP 0x8000

typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;

typedef struct Game Game;

typedef struct Camera Camera;
struct Camera {
    Coord co;
};

typedef struct RenderMap RenderMap;
struct RenderMap {
    CE_u16 tiles_width;
    CE_u16 tiles_height;
    CE_u16 map_width;
    CE_u16 map_height;
    SDL_Texture *tilesheet;
    CE_u16 *tile_map;
};

// null texture is flag for inactive sprite
typedef struct Sprite Sprite;
struct Sprite {
    CE_u32 frame;
    CE_u32 frame_max;
    CE_u32 animation;
    CE_u32 current_delay;
    CE_u32 frame_delay;
    CE_u32 is_playing;
    SDL_Texture *texture;
    Coord co;
};

typedef struct SpriteQueue SpriteQueue;
struct SpriteQueue {
    CE_u32 start, end;
    SpriteHandle queue[MAX_ENTITIES];
};

enum TBoxState {
    TBOX_CLOSED,
    TBOX_STATIC,
    TBOX_SCROLLING,
};

typedef struct TextBox TextBox;
struct TextBox {
    CE_i32 scroll;
    enum TBoxState state;
    CE_i32 line;
    SDL_Rect box;
    char rows[TBOX_ROWS][TBOX_LIMIT];
};

typedef struct Curtain Curtain;
struct Curtain {
    CE_i32 height;
    CE_i32 target_height;
};

typedef struct Renderer Renderer;
typedef void (*RenderHookFn) (Renderer *, void *);

struct Renderer {
    CE_u8 map_disable;
    CE_u8 sprite_disable;
    CE_u8 tbox_disable;
    CE_u8 camera_disable;
    CE_u8 animations_disable;
    CE_u8 camera_lock;
    int prompt_state;
    int quake_time;
    int quake_dir;
    int quake_offset;
    SDL_Renderer *renderer;
    SDL_Texture *canvas;
    RenderHookFn hook;
    void *hook_data;
    SDL_Color base;
    SDL_Color accent;
    Curtain curtain;
    Camera camera;
    RenderMap map;
    SpriteQueue sprite_queue;
    Sprite sprite_pool[MAX_ENTITIES];
    TextBox text_box;
};

int RendererInit(Renderer *renderer, SDL_Window *window);
void RendererDeinit(Renderer *renderer);
void RendererUpdate(Renderer *renderer);
void RendererDraw(Renderer *renderer);
void RendererSetHook(Renderer *renderer, RenderHookFn func, void *data);

void RendererEnableMap(Renderer *renderer);
void RendererEnableSprite(Renderer *renderer);
void RendererEnableTextBox(Renderer *renderer);
void RendererEnableCamera(Renderer *renderer);
void RendererEnableCameraLock(Renderer *renderer);
void RendererEnableAnimations(Renderer *renderer);

void RendererDisableMap(Renderer *renderer);
void RendererDisableSprite(Renderer *renderer);
void RendererDisableTextBox(Renderer *renderer);
void RendererDisableCamera(Renderer *renderer);
void RendererDisableCameraLock(Renderer *renderer);
void RendererDisableAnimations(Renderer *renderer);

void SetCamera(Renderer *renderer, Coord co);
void SetQuakeTime(Renderer *renderer, int time);

SpriteHandle SpriteLoad(Game *game, const char *name, CE_u32 frame_delay);
SpriteHandle SpriteNew(Renderer *renderer, SDL_Texture *texture, CE_u32 frame_delay);
void SpriteFree(Renderer *renderer, SpriteHandle id);
void SpriteDraw(Renderer *renderer, SpriteHandle id, Coord co);
void SpriteSetAnimation(Renderer *renderer, SpriteHandle id, CE_u32 animation);
void SpriteStartAnimation(Renderer *renderer, SpriteHandle id);
void SpriteStopAnimation(Renderer *renderer, SpriteHandle id);
void SpriteResetAnimation(Renderer *renderer, SpriteHandle id);

void TextBoxOpenTop(Renderer *renderer);
void TextBoxOpenBottom(Renderer *renderer);
void TextBoxClose(Renderer *renderer);
void TextBoxClear(Renderer *renderer);
void TextBoxScroll(Renderer *renderer);
void TextBoxNewLine(Renderer *renderer);
void TextBoxAppend(Renderer *renderer, String text);
int TextBoxIsOpen(Renderer *renderer);
int TextBoxIsScrolling(Renderer *renderer);

void PromptDraw(Renderer *renderer, int choice);

void CurtainRaise(Renderer *renderer);
void CurtainFall(Renderer *renderer);
int CurtainIsDown(Renderer *renderer);
int CurtainInMotion(Renderer *renderer);

#endif
