#include <stdio.h>
#include <cose/cose.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#define LOG_FILE "tiles.log"

#include "../src/config.h"
#include "../src/common.h"
#include "../src/game.h"
#include "../src/log.h"
#include "../src/render.h"
#include "../src/map.h"
#include "../src/entity.h"
#include "../src/cutscene.h"
#include "../src/script.h"
#include "../src/sound.h"
#include "../src/font.h"

#include "../src/game.c"
#include "../src/log.c"
#include "../src/render.c"
#include "../src/map.c"
#include "../src/entity.c"
#include "../src/cutscene.c"
#include "../src/script.c"
#include "../src/sound.c"

#define NIL_ENTITY 0x80000000

typedef struct RawEntity RawEntity;
struct RawEntity {
    CE_u32 type;
    CE_u32 x;
    CE_u32 y;
    CE_u32 event;
};

typedef struct RawMap RawMap;
struct RawMap {
    CE_u32 magic;
    CE_u32 width;
    CE_u32 height;
    CE_u32 entity_count;
    CE_u32 tiles_count;
    CE_u32 base;
    CE_u32 accent;
    char tilesheet[16];
    
    CE_u16 *tile_map;
    RawEntity *entities;
    CE_u8 *tile_types;
};

int LoadRawMap(CE_Arena *arena, RawMap *map, const char *path)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        AlertError("Resource Error", "Could not open map file '%s'", path);
        return 0;
    }

    fread(&map->magic, 4, 1, file);
    
    if (map->magic != MAP_MAGIC) {
        AlertError("Resource Error", "Map file '%s' has no magic", path);
        return 0;
    }

    fread(&map->width, 4, 1, file);
    fread(&map->height, 4, 1, file);
    fread(map->tilesheet, 1, 16, file);
    fread(&map->entity_count, 4, 1, file);
    fread(&map->tiles_count, 4, 1, file);
    fread(&map->base, 4, 1, file);
    fread(&map->accent, 4, 1, file);

    map->tile_map = CE_ArenaPush(arena, sizeof(CE_u16) * map->width * map->height);
    map->entities = CE_ArenaPush(arena, ENTITY_FILE_SIZE * map->entity_count);
    map->tile_types = CE_ArenaPush(arena, map->tiles_count);
    
    int read = fread(map->tile_map, 2, map->width * map->height, file);
    if (read != map->width * map->height) {
        AlertError("Resource Error", "Map file '%s' low read on map", path);
        return 0;
    }
    read = fread(map->entities, ENTITY_FILE_SIZE, map->entity_count, file);
    if (read != map->entity_count) {
        AlertError("Resource Error", "Map file '%s' low read on entities", path);
        return 0;
    }
    read = fread(map->tile_types, 1, map->tiles_count, file);
    if (read != map->tiles_count) {
        AlertError("Resource Error", "Map file '%s' low read on tiles", path);
        return 0;
    }

    fclose(file);
    
    return 1;
}

int WriteRawMap(RawMap *map, const char *path)
{
    FILE *file = fopen(path, "wb");
    if (!file) {
        LogError("No open file!");
        return 0;
    }

    if (map->entity_count > MAX_ENTITIES) {
        fclose(file);
        LogError("Entity threshold exceeded (%u)", map->entity_count);
        return 0;
    }
    
    fwrite(&map->magic, 4, 1, file);
    fwrite(&map->width, 4, 1, file);
    fwrite(&map->height, 4, 1, file);
    fwrite(map->tilesheet, 1, 16, file);
    fwrite(&map->entity_count, 4, 1, file);
    fwrite(&map->tiles_count, 4, 1, file);
    fwrite(&map->base, 4, 1, file);
    fwrite(&map->accent, 4, 1, file);

    fwrite(map->tile_map, 2, map->width * map->height, file);
    fwrite(map->entities, ENTITY_FILE_SIZE, map->entity_count, file);
    fwrite(map->tile_types, 1, map->tiles_count, file);

    return 1;
}

int LoadRenderMap(CE_Arena *arena, Renderer *renderer, RawMap *map)
{
    CE_u64 size = snprintf(0, 0, "res/tilesheets/%.16s.png", map->tilesheet);
    char *tilesheet_path = CE_ArenaPush(arena, size + 1);
    snprintf(tilesheet_path, size+1, "res/tilesheets/%.16s.png", map->tilesheet);

    SDL_Surface *tmp = IMG_Load(tilesheet_path);
    if (!tmp) {
        AlertError("Resource Error", "Could not load tilesheet '%s': %s", tilesheet_path, IMG_GetError());
        return 0;
    }
    renderer->map.tilesheet = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
    SDL_FreeSurface(tmp);
    if (!renderer->map.tilesheet) {
        AlertError("SDL Error", "Could not create texture from surface: %s", SDL_GetError());
        return 0;
    }

    int width, height;
    SDL_QueryTexture(renderer->map.tilesheet, 0, 0, &width, &height);
    width /= TILE;
    height /= TILE;
    renderer->map.tiles_width = width;
    renderer->map.tiles_height = height;
    renderer->map.map_width = map->width;
    renderer->map.map_height = map->height;

    renderer->map.tile_map = map->tile_map;

    return 1;
}

Coord ScreenToMap(Vec2 screen_pos, Coord camera)
{
    screen_pos.x /= SCALE;
    screen_pos.y /= SCALE;
    screen_pos.x -= WIDTH / 2 - TILE / 2;
    screen_pos.x += camera.position.x * TILE + camera.offset.x;
    screen_pos.y -= HEIGHT / 2 - TILE / 2;
    screen_pos.y += camera.position.y * TILE + camera.offset.y;

    Coord result;
    result.position.x = screen_pos.x / TILE;
    result.position.y = screen_pos.y / TILE;
    result.offset.x = screen_pos.x % TILE;
    result.offset.y = screen_pos.y % TILE;

    return result;
}

enum EditorState {
    EDITOR_NORMAL,
    EDITOR_NUMBER,
};

typedef struct Editor Editor;
struct Editor {
    Game *game;
    RawEntity entities[MAX_ENTITIES];
    RawEntity *active;
    int state;
    char number[5];
    Coord camera;
    Vec2 selection;
    SDL_Color color;
};

char *table[] = {
    "ET_TRIGGER",
    "ET_FRUIT",
    "ET_LEVER",
    "ET_DOOR",
    "ET_CAVE_ORB",
    "ET_RUINS_ORB",
    "ET_GHOST",
    "ET_SWORD",
    "ET_BUG",
    "ET_WORM",
    "ET_SKULL",
    "ET_MOOK",
    "ET_SEER",
    "ET_PLAYER",
};

static void render_clicks(Renderer *renderer, void *data) {
    Editor *e = data;

    for (int i = 0; i < MAX_ENTITIES; ++i) {
        if (e->entities[i].type != NIL_ENTITY) {
            Coord pos = {{e->entities[i].x, e->entities[i].y}, {0}};
            SDL_Rect rect = CameraTransform(renderer, pos);
            SDL_SetRenderDrawColor(renderer->renderer, 0, 0xff, 0, 0xff);
            SDL_RenderDrawRect(renderer->renderer, &rect);
            
            // type #
            char type[3];
            snprintf(type, 3, "%u", e->entities[i].type);
            SDL_Surface *tmp = RenderText(type, e->color, renderer->base);
            rect.x += 1;
            rect.y += 1;
            rect.w = tmp->w;
            rect.h = tmp->h;
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
            SDL_FreeSurface(tmp);
            SDL_RenderCopy(renderer->renderer, tex, 0, &rect);
            SDL_DestroyTexture(tex);
        }
    }

    Coord pos = {e->selection, {0}};
    SDL_Rect rect = CameraTransform(renderer, pos);
    SDL_SetRenderDrawColor(renderer->renderer, 0xff, 0xff, 0, 0xff);
    SDL_RenderDrawRect(renderer->renderer, &rect);
    
    if (e->active) {
        pos = (Coord) {{e->active->x, e->active->y}, {0}};
        rect = CameraTransform(renderer, pos);
        SDL_SetRenderDrawColor(renderer->renderer, 0xff, 0, 0, 0xff);
        SDL_RenderDrawRect(renderer->renderer, &rect);

        char *type_name;
        if (e->active->type < ET_LAST) {
            type_name = table[e->active->type];
        } else {
            type_name = "ET_UNKOWN";
        }

        CE_u64 size = snprintf(0, 0, "%s (%u)", type_name, e->active->type);
        char *type = CE_ArenaPush(e->game->frame_arena, size+1);
        snprintf(type, size+1, "%s (%u)", type_name, e->active->type);
        
        SDL_Surface *tmp = RenderText(type, e->color, renderer->base);
        rect = (SDL_Rect) {0, 2, tmp->w, tmp->h};
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
        SDL_FreeSurface(tmp);
        SDL_RenderCopy(renderer->renderer, tex, 0, &rect);
        SDL_DestroyTexture(tex);
        
        if (e->state == EDITOR_NORMAL) {
            snprintf(type, size+1, "Event: %04u", e->active->event);
        } else {
            snprintf(type, size+1, "Event: %s", e->number);
        }
        tmp = RenderText(type, e->color, renderer->base);
        rect = (SDL_Rect) {0, 10, tmp->w, tmp->h};
        tex = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
        SDL_FreeSurface(tmp);
        SDL_RenderCopy(renderer->renderer, tex, 0, &rect);
        SDL_DestroyTexture(tex);
    }
    CE_u64 size = 32;
    char *text = CE_ArenaPush(e->game->frame_arena, size + 1);
    snprintf(text, size+1, "X: %d", e->selection.x);
    SDL_Surface *tmp = RenderText(text, e->color, renderer->base);
    rect = (SDL_Rect) {120, 2, tmp->w, tmp->h};
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
    SDL_FreeSurface(tmp);
    SDL_RenderCopy(renderer->renderer, tex, 0, &rect);
    SDL_DestroyTexture(tex);
    snprintf(text, size+1, "Y: %d", e->selection.y);
    tmp = RenderText(text, e->color, renderer->base);
    rect = (SDL_Rect) {120, 10, tmp->w, tmp->h};
    tex = SDL_CreateTextureFromSurface(renderer->renderer, tmp);
    SDL_FreeSurface(tmp);
    SDL_RenderCopy(renderer->renderer, tex, 0, &rect);
    SDL_DestroyTexture(tex);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <map_file>\n", argv[0]);
        return 1;
    }

    char *in_file = argv[1];
    
    Game game;
    RawMap rmap = {0};
    Editor editor = {0};
    
    editor.game = &game;
    for (int i = 0; i < MAX_ENTITIES; ++i) {
        editor.entities[i].type = NIL_ENTITY;
    }
    
    if (!GameInit(&game)) {
        return 1;
    }

    if (!LoadRawMap(game.global, &rmap, in_file)) {
        return 1;
    }

    if (!LoadRenderMap(game.map_arena, game.renderer, &rmap)) {
        return 1;
    }

    // Load RawMap entities into Editor
    memcpy(editor.entities, rmap.entities, rmap.entity_count * ENTITY_FILE_SIZE);

    RendererDisableCameraLock(game.renderer);
    RendererSetHook(game.renderer, &render_clicks, &editor);

    int running = 1;
    while (running) {
        SDL_Event event;
        Vec2 mouse_pos;
        CE_u32 mouse_button = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN && editor.state == EDITOR_NUMBER) {
                SDL_Keycode key = event.key.keysym.sym;
                CE_u64 length = strlen(editor.number);
                if (key >= SDLK_0 && key <= SDLK_9 && length < 4) {
                    editor.number[length] = key;
                } else if (key == SDLK_RETURN) {
                    editor.state = EDITOR_NORMAL;
                    CE_u32 number;
                    int count = sscanf(editor.number, "%u", &number);
                    if (count == 1) {
                        editor.active->event = number;
                    }
                    memset(editor.number, 0, 5);
                } else if (key == SDLK_BACKSPACE && length > 0) {
                    editor.number[length-1] = 0;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_r) {
                    editor.camera = (Coord) {0};
                }
                if (event.key.keysym.sym == SDLK_a) {
                    editor.camera.position.x -= 1;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    editor.camera.position.x += 1;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    editor.camera.position.y -= 1;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    editor.camera.position.y += 1;
                }
                if (event.key.keysym.sym == SDLK_u) {
                    // CE_u64 size = snprintf(0, 0, "%s.bak", in_file);
                    // char *path = CE_ArenaPush(game.frame_arena, size+1);
                    // snprintf(path, size+1, "%s.bak", in_file);
                    char *path = in_file;

                    // Load entities into RawMap
                    rmap.entity_count = 0;
                    rmap.entities = CE_ArenaPush(game.frame_arena, MAX_ENTITIES * ENTITY_FILE_SIZE);
                    for (int i = 0; i < MAX_ENTITIES; ++i) {
                        if (editor.entities[i].type != NIL_ENTITY) {
                            rmap.entities[rmap.entity_count] = editor.entities[i];
                            rmap.entity_count += 1;
                        }
                    }
                    
                    LogMsg("Writing to %s", path);
                    WriteRawMap(&rmap, path);
                }
                if (event.key.keysym.sym == SDLK_i) {
                    int i;
                    for (i = 0; i < MAX_ENTITIES; ++i) {
                        if (editor.entities[i].type == NIL_ENTITY) {
                            break;
                        }
                    }
                    if (i < MAX_ENTITIES) {
                        editor.entities[i].type = 0;
                        editor.entities[i].x = editor.selection.x;
                        editor.entities[i].y = editor.selection.y;
                        editor.active = &editor.entities[i];
                    }
                }
                if (event.key.keysym.sym == SDLK_c) {
                    editor.active = 0;
                }
                if (event.key.keysym.sym == SDLK_m && editor.active) {
                    editor.active->x = editor.selection.x;
                    editor.active->y = editor.selection.y;
                }
                if (event.key.keysym.sym == SDLK_x && editor.active) {
                    editor.active->type = NIL_ENTITY;
                    editor.active = 0;
                }
                if (event.key.keysym.sym == SDLK_v && editor.active) {
                    if (editor.active->type > 0) {
                        editor.active->type -= 1;
                    }
                }
                if (event.key.keysym.sym == SDLK_b && editor.active) {
                    if (editor.active->type < ET_LAST-1) {
                        editor.active->type += 1;
                    }
                }
                if (event.key.keysym.sym == SDLK_n && editor.active) {
                    editor.state = EDITOR_NUMBER;
                }
                if (event.key.keysym.sym == SDLK_f) {
                    editor.camera.position = (Vec2) {rmap.width / 2, rmap.height / 2};
                }
                if (event.key.keysym.sym == SDLK_z) {
                    editor.color.r = ~editor.color.r;
                    editor.color.g = ~editor.color.g;
                    editor.color.b = ~editor.color.b;
                }
            }
        }

        Coord click = ScreenToMap(mouse_pos, game.renderer->camera.co);
        if ((SDL_BUTTON(1) & mouse_button) && editor.state == EDITOR_NORMAL) {
            editor.selection = click.position;
            for (int i = 0; i < MAX_ENTITIES; ++i) {
                if (editor.entities[i].type != NIL_ENTITY &&
                        editor.entities[i].x == click.position.x &&
                        editor.entities[i].y == click.position.y) {
                    editor.active = &editor.entities[i];
                    break;
                }
            }
        }

        SetCamera(game.renderer, editor.camera);
        RendererDraw(game.renderer);

        CE_ArenaClear(game.frame_arena);
        SDL_Delay(10);
    }

    GameDeinit(&game);

    return 0;
}
