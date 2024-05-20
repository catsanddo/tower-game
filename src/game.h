#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef struct SDL_Window SDL_Window;
typedef struct CE_Arena CE_Arena;

typedef struct Renderer Renderer;
typedef struct Map Map;
typedef struct EntityPool EntityPool;
typedef struct Script Script;
typedef struct SoundEngine SoundEngine;

typedef struct Input Input;
struct Input {
    bool left;
    bool right;
    bool up;
    bool down;
    bool left_pressed;
    bool right_pressed;
    bool up_pressed;
    bool down_pressed;

    bool action;
    bool cancel;
    bool start;
    bool select;
    bool action_pressed;
    bool cancel_pressed;
    bool start_pressed;
    bool select_pressed;

    bool quit;
};

typedef struct Clock Clock;
struct Clock {
    CE_u32 frames;
    CE_u32 uncapped_fps;
    CE_u64 game_time;
    CE_i64 delta_time;
};

typedef struct Game Game;
typedef int (*CutSceneFn) (Game *);
struct Game {
    bool input_lock;
    bool triggered;
    SDL_Window *window;
    
    CE_Arena *global;
    CE_Arena *map_arena;
    CE_Arena *frame_arena;
    
    Renderer *renderer;
    Map *map;
    EntityPool *entity_pool;
    Script *script;
    SoundEngine *sengine;
    
    CE_u8 *global_flags;
    
    CutSceneFn cutscene;
    void *cutscene_data;
    
    Clock clock;
    Input input;
};

int GameInit(Game *g);
void GameDeinit(Game *g);
void GrabInput(Game *g);
void Tick(Game *g);

int GetCollision(Game *g, Vec2 position);

int GlobalFlagQuery(Game *g, CE_u32 flag);
void GlobalFlagSet(Game *g, CE_u32 flag);
void GlobalFlagReset(Game *g, CE_u32 flag);

#endif
