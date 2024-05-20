#include "map.h"
#include "game.h"
#include "log.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

const char *map_names[] = {
    "Beach",
    "Cave1",
    "Cave2",
    "Forest",
    "Ruins",
    "Swamp",
    "Test",
    "Tower Cave",
    "Tower Entrance",
    "Tower Top",
    "Tower",
    "Town",
};

int MapLoad(Game *game, const char *name)
{
    Map *map = game->map;
    FILE *file;
    char *map_path;
    char *tilesheet_path;
    char *script_path;
    char tilesheet[16];
    CE_u8 base_color[4];
    CE_u8 accent_color[4];
    CE_u64 length;
    CE_u32 magic;
    CE_u32 entity_count;

    length = snprintf(0, 0, "res/maps/%s.cmp", name);
    map_path = CE_ArenaPush(game->map_arena, length + 1);
    snprintf(map_path, length + 1, "res/maps/%s.cmp", name);

    file = fopen(map_path, "rb");
    if (!file) {
        AlertError("Resource Error", "Could not open map file: '%s'", map_path);
        return 0;
    }

    fread(&magic, 4, 1, file);
    if (magic != MAP_MAGIC) {
        AlertError("Resource Error", "Map file lacks magic: '%s'", map_path);
        return 0;
    }

    fread(&map->width, 4, 1, file);
    fread(&map->height, 4, 1, file);
    fread(tilesheet, 1, 16, file);
    fread(&entity_count, 4, 1, file);
    fread(&map->tiles_count, 4, 1, file);
    fread(base_color, 1, 4, file);
    fread(accent_color, 1, 4, file);

    if (map->tiles_count > MAX_TILES) {
        AlertError("Resource Error", "Tilesheet too large: '%.*s'", 16, tilesheet);
        return 0;
    }

    CE_u64 map_size = map->width * map->height;
    map->tile_map = CE_ArenaPush(game->map_arena,
            sizeof(CE_u16) * map_size);
    if (fread(map->tile_map, 2, map_size, file) != map_size) {
        AlertError("Resource Error", "Map corruption likely (1): '%s'", map_path);
        return 0;
    }

    for (CE_u64 i = 0; i < entity_count; ++i) {
        CE_u32 type;
        CE_u32 x;
        CE_u32 y;
        CE_u32 event_id;

        fread(&type, 4, 1, file);
        fread(&x, 4, 1, file);
        fread(&y, 4, 1, file);
        fread(&event_id, 4, 1, file);
        
        EntityNew(game, type, (Vec2) {x, y}, event_id);
    }

    map->tile_types = CE_ArenaPush(game->map_arena, map->tiles_count);
    if (fread(map->tile_types, 1, map->tiles_count, file) != map->tiles_count) {
        AlertError("Resource Error", "Map corruption likely (2): '%s'", map_path);
        return 0;
    }

    fclose(file);

    // done reading; now onto other stuff
    game->renderer->base.r = base_color[0];
    game->renderer->base.g = base_color[1];
    game->renderer->base.b = base_color[2];
    game->renderer->base.a = 0xff;
    game->renderer->accent.r = accent_color[0];
    game->renderer->accent.g = accent_color[1];
    game->renderer->accent.b = accent_color[2];
    game->renderer->base.a = 0xff;

    length = snprintf(0, 0, "res/tilesheets/%.16s.png", tilesheet);
    tilesheet_path = CE_ArenaPush(game->map_arena, length + 1);
    snprintf(tilesheet_path, length + 1, "res/tilesheets/%.16s.png", tilesheet);

    // fill out RenderMap
    game->renderer->map.tile_map = map->tile_map;
    SDL_Surface *tmp = IMG_Load(tilesheet_path);
    if (!tmp) {
        AlertError("Resource Error", "Could not open tilesheet '%s': '%s'", tilesheet_path, IMG_GetError());
        return 0;
    }
    game->renderer->map.tiles_width = tmp->w / TILE;
    game->renderer->map.tiles_height = tmp->h / TILE;
    game->renderer->map.tilesheet = SDL_CreateTextureFromSurface(game->renderer->renderer, tmp);
    SDL_FreeSurface(tmp);
    if (!game->renderer->map.tilesheet) {
        AlertError("SDL Error", "Could not texturize tilesheet '%s': '%s'", tilesheet_path, SDL_GetError());
        return 0;
    }
    game->renderer->map.map_width = map->width;
    game->renderer->map.map_height = map->height;

    // Load map script
    length = snprintf(0, 0, "res/scripts/%s.csc", name);
    script_path = CE_ArenaPush(game->map_arena, length + 1);
    length = snprintf(script_path, length + 1, "res/scripts/%s.csc", name);
    game->script->buffer.len = 0;
    if (!ScriptLoad(game, script_path)) {
        return 0;
    }

    return 1;
}

void MapUnload(Game *game)
{
    Map *map = game->map;

    SDL_DestroyTexture(game->renderer->map.tilesheet);
    game->renderer->map.tilesheet = 0;

    *map = (Map) {0};
    CE_ArenaClear(game->map_arena);

    EntityForEach(entity) {
        EntityFree(game, entity);
    }
}
