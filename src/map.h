#ifndef MAP_H
#define MAP_H

#define MAP_MAGIC 0x5a504d43
#define MAX_TILES 0x7fff
#define ENTITY_FILE_SIZE 16

typedef struct Game Game;

extern const char *map_names[];

typedef struct Map Map;
struct Map {
    CE_u32 width;
    CE_u32 height;
    CE_u32 tiles_count;
    CE_u8 *tile_types;
    CE_u16 *tile_map;
};

int MapLoad(Game *game, const char *name);
void MapUnload(Game *game);

#endif
