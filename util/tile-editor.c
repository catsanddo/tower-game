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
    CE_u32 *entities;
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

    // CE_u16 *tile_map;
    int width, height;
    SDL_QueryTexture(renderer->map.tilesheet, 0, 0, &width, &height);
    width /= TILE;
    height /= TILE;
    renderer->map.tiles_width = width;
    renderer->map.tiles_height = height;
    renderer->map.map_width = width;
    renderer->map.map_height = height;

    renderer->map.tile_map = CE_ArenaPush(arena,
            sizeof(CE_u16) * width * height);

    for (CE_u16 i = 0; i < width * height; ++i) {
        renderer->map.tile_map[i] = i+1;
    }

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

static void render_clicks(Renderer *renderer, void *data) {
    RawMap *map = data;
    SDL_SetRenderDrawBlendMode(renderer->renderer, SDL_BLENDMODE_BLEND);
    for (int y = 0; y < renderer->map.tiles_height; ++y) {
        for (int x = 0; x < renderer->map.tiles_width; ++x) {
            if (!map->tile_types[y*renderer->map.tiles_width+x]) {
                continue;
            }
            Coord pos = {{x, y}, {0}};
            SDL_Rect rect = CameraTransform(renderer, pos);
            SDL_SetRenderDrawColor(renderer->renderer, 0xff, 0, 0, 0x80);
            SDL_RenderFillRect(renderer->renderer, &rect);
        }
    }
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
    if (!GameInit(&game)) {
        return 1;
    }

    if (!LoadRawMap(game.global, &rmap, in_file)) {
        return 1;
    }

    if (!LoadRenderMap(game.map_arena, game.renderer, &rmap)) {
        return 1;
    }

    RendererDisableCameraLock(game.renderer);
    RendererSetHook(game.renderer, &render_clicks, &rmap);

    Coord camera = {0};
    int running = 1;
    while (running) {
        SDL_Event event;
        Vec2 mouse_pos;
        CE_u32 mouse_button = SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_r) {
                    camera = (Coord) {0};
                }
                if (event.key.keysym.sym == SDLK_a) {
                    camera.position.x -= 1;
                }
                if (event.key.keysym.sym == SDLK_d) {
                    camera.position.x += 1;
                }
                if (event.key.keysym.sym == SDLK_w) {
                    camera.position.y -= 1;
                }
                if (event.key.keysym.sym == SDLK_s) {
                    camera.position.y += 1;
                }
                if (event.key.keysym.sym == SDLK_u) {
                    // CE_u64 size = snprintf(0, 0, "%s.bak", in_file);
                    // char *path = CE_ArenaPush(game.frame_arena, size+1);
                    // snprintf(path, size+1, "%s.bak", in_file);
                    char *path = in_file;
                    
                    LogMsg("Writing to %s", path);
                    WriteRawMap(&rmap, path);
                }
            }
        }

        int button = -1;
        if (SDL_BUTTON(1) & mouse_button) {
            button = 1;
        } else if (SDL_BUTTON(3) & mouse_button) {
            button = 0;
        }

        Coord click = ScreenToMap(mouse_pos, game.renderer->camera.co);
        if (button >= 0 && click.position.x < game.renderer->map.tiles_width &&
                click.position.y < game.renderer->map.tiles_height) {
            CE_u32 i = click.position.y * game.renderer->map.tiles_width + click.position.x;
            rmap.tile_types[i] = button;
        }

        SetCamera(game.renderer, camera);
        RendererDraw(game.renderer);

        CE_ArenaClear(game.frame_arena);
        SDL_Delay(10);
    }

    GameDeinit(&game);

    return 0;
}
