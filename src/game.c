#include "game.h"
#include "render.h"
#include "map.h"
#include "entity.h"
#include "script.h"
#include "sound.h"
#include "config.h"
#include "log.h"
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

static int CheckLittleEndian(void)
{
    CE_u32 value = 1;
    char bytes[4];
    memcpy(bytes, &value, 4);

    return bytes[0];
}

int GameInit(Game *g)
{
    *g = (Game) {0};
    
    if (!CheckLittleEndian()) {
        AlertError("Lazy Error", "This game is not supported on big endian platforms. Go complain to the dev and he will fix it. He was just being lazy.");
        return 0;
    }
    
    CE_u32 sdl_flags = SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO |
            SDL_INIT_EVENTS;
    if (SDL_Init(sdl_flags) < 0) {
        AlertError("SDL Error", "Initializing: %s", SDL_GetError());
        return 0;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) {
        AlertError("SDL_image Error", "Loading IMG_INIT_PNG: %s", IMG_GetError());
        SDL_Quit();
        return 0;
    }

    if (TTF_Init() < 0) {
        AlertError("SDL_ttf Error", "Loading SDL_ttf functions: %s", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    g->window = SDL_CreateWindow("Tower Game",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (g->window == 0) {
        AlertError("SDL Error", "Opening window: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    g->global = CE_ArenaAlloc();
    g->map_arena = CE_ArenaAlloc();
    g->frame_arena = CE_ArenaAlloc();
        
    g->renderer = CE_ArenaPush(g->global, sizeof(Renderer));
    if (!RendererInit(g->renderer, g->window)) {
        return 0;
    }

    g->script = CE_ArenaPush(g->global, sizeof(Script));
    if (!ScriptInit(g, g->script)) {
        AlertError("Script Error", "Could not initialize script engine");
        return 0;
    }

    g->sengine = CE_ArenaPush(g->global, sizeof(SoundEngine));
    if (!SoundEngineInit(g->global, g->sengine)) {
        return 0;
    }

    g->map = CE_ArenaPush(g->global, sizeof(Map));
    *(g->map) = (Map) {0};

    g->entity_pool = CE_ArenaPush(g->global, sizeof(EntityPool));
    memset(g->entity_pool, 0, sizeof(EntityPool));

    g->global_flags = CE_ArenaPush(g->global, FLAG_LIMIT / 8);
    memset(g->global_flags, 0, FLAG_LIMIT / 8);

    g->clock.game_time = SDL_GetTicks64();

    return 1;
}

void GameDeinit(Game *g)
{
    RendererDeinit(g->renderer);
    SoundEngineDeinit(g->sengine);
    
    CE_ArenaFree(g->global);
    CE_ArenaFree(g->map_arena);
    CE_ArenaFree(g->frame_arena);

    TTF_Quit();
    IMG_Quit();
    
    SDL_DestroyWindow(g->window);
    SDL_Quit();

}

void GrabInput(Game *g)
{
    SDL_Event event;

    g->input.action_pressed = 0;
    g->input.cancel_pressed = 0;
    g->input.start_pressed = 0;
    g->input.select_pressed = 0;
    g->input.left_pressed = 0;
    g->input.right_pressed = 0;
    g->input.up_pressed = 0;
    g->input.down_pressed = 0;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            g->input.quit = 1;
        } else if (event.type == SDL_KEYDOWN && !event.key.repeat) {
            if (event.key.keysym.sym == SDLK_z) {
                g->input.cancel = 1;
                g->input.cancel_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_x) {
                g->input.action = 1;
                g->input.action_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_RSHIFT) {
                g->input.select = 1;
                g->input.select_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_RETURN) {
                g->input.start = 1;
                g->input.start_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_LEFT) {
                g->input.left = 1;
                g->input.left_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_RIGHT) {
                g->input.right = 1;
                g->input.right_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_UP) {
                g->input.up = 1;
                g->input.up_pressed = 1;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                g->input.down = 1;
                g->input.down_pressed = 1;
            }
        } else if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == SDLK_z) {
                g->input.cancel = 0;
            } else if (event.key.keysym.sym == SDLK_x) {
                g->input.action = 0;
            } else if (event.key.keysym.sym == SDLK_RSHIFT) {
                g->input.select = 0;
            } else if (event.key.keysym.sym == SDLK_RETURN) {
                g->input.start = 0;
            } else if (event.key.keysym.sym == SDLK_LEFT) {
                g->input.left = 0;
            } else if (event.key.keysym.sym == SDLK_RIGHT) {
                g->input.right = 0;
            } else if (event.key.keysym.sym == SDLK_UP) {
                g->input.up = 0;
            } else if (event.key.keysym.sym == SDLK_DOWN) {
                g->input.down = 0;
            }
        }
    }

    // TODO: remove hack
    if (!g->input_lock && g->input.select_pressed) {
        MapUnload(g);
        if (!MapLoad(g, "Test")) {
            g->input.quit = 1;
            return;
        }
        Vec2 pos = {5, 7};
        EntityNew(g, ET_PLAYER, pos, 0);
        PlayTrack(g->sengine, g->frame_arena, 0);
    }
}

void Tick(Game *g)
{
    g->clock.frames += 1;
    CE_u64 ticks = SDL_GetTicks64();
    
    g->clock.delta_time = ticks - g->clock.game_time;

    if (g->clock.delta_time) {
        g->clock.uncapped_fps = 1000 / g->clock.delta_time;
    }
    
    CE_i64 delay = 1000 / MAX_FPS - g->clock.delta_time;
    if (delay > 0) {
        SDL_Delay(delay);
    }
    
    g->clock.game_time = SDL_GetTicks64();
}

int GetCollision(Game *g, Vec2 position)
{
    int result = 0;
    
    EntityForEach(entityID) {
        if (EntityQueryFlags(g, entityID, ENTITY_SOLID)) {
            Entity *entity = EntityGet(g, entityID);
            if (entity->co.position.x == position.x &&
                    entity->co.position.y == position.y) {
                return 1;
            }
        }
    }

    if (position.x >= 0  && position.x < g->map->width &&
            position.y >= 0  && position.y < g->map->height) {
        CE_u32 i = position.y * g->map->width + position.x;
        CE_u32 tile = g->map->tile_map[i] & (~0x8000);
        if (tile == 0) {
            return 0;
        }
        result = g->map->tile_types[tile-1];
    }

    return result;
}

int GlobalFlagQuery(Game *g, CE_u32 flag)
{
    if (flag >= FLAG_LIMIT) {
        LogWarning("Flag exceeds limits (%u)", flag);
        return 0;
    }

    CE_u32 byte_offset = flag >> 3;
    CE_u32 bit_offset = flag & 7;
    
    int flag_status = (g->global_flags[byte_offset] & (1 << bit_offset)) != 0;

    return flag_status;
}

void GlobalFlagSet(Game *g, CE_u32 flag)
{
    if (flag >= FLAG_LIMIT) {
        LogWarning("Flag exceeds limits (%u)", flag);
        return;
    }

    CE_u32 byte_offset = flag >> 3;
    CE_u32 bit_offset = flag & 7;
    
    g->global_flags[byte_offset] |= 1 << bit_offset;
}

void GlobalFlagReset(Game *g, CE_u32 flag)
{
    if (flag >= FLAG_LIMIT) {
        LogWarning("Flag exceeds limits (%u)", flag);
        return;
    }

    CE_u32 byte_offset = flag >> 3;
    CE_u32 bit_offset = flag & 7;
    
    g->global_flags[byte_offset] &= ~(1 << bit_offset);
}
