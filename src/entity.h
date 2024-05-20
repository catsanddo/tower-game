#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "config.h"

#define EntityForEach(s) for (EntityHandle s = {0}; s.handle < MAX_ENTITIES; s.handle += 1)

enum EntityFlags {
    ENTITY_ACTIVE       = 1 << 0,
    ENTITY_VISIBLE      = 1 << 1,
    ENTITY_PLAYER       = 1 << 2,
    ENTITY_SOLID        = 1 << 3,
    ENTITY_INTERACTABLE = 1 << 4,
    ENTITY_TRIGGER      = 1 << 5,
    ENTITY_ANIMATE      = 1 << 6,
};

typedef struct Entity Entity;
struct Entity {
    CE_u64 flags;
    CE_u64 eventID;
    SpriteHandle sprite;
    Vec2 momentum;
    Coord co;
    CE_u32 direction;
};

typedef struct EntityPool EntityPool;
struct EntityPool {
    Entity pool[MAX_ENTITIES];
};

enum EntityTable {
    ET_TRIGGER,
    ET_FRUIT,
    ET_LEVER,
    ET_DOOR,
    ET_CAVE_ORB,
    ET_RUINS_ORB,
    ET_GHOST,
    ET_SWORD,
    ET_BUG,
    ET_WORM,
    ET_SKULL,
    ET_MOOK,
    ET_SEER,
    ET_PLAYER,
    ET_LAST,
};

typedef struct EntityTemplate EntityTemplate;
struct EntityTemplate {
    char *sprite_name;
    CE_u32 frame_delay;
    CE_u64 flags;
};

extern const EntityTemplate entity_table[];

EntityHandle EntityNew(Game *game, CE_u64 index, Vec2 position, CE_u64 eventID);
EntityHandle EntityConstruct(Game *game, Vec2 position, const char *sprite_name,
        CE_u32 frame_delay, CE_u64 eventID, CE_u64 flags);
void EntityFree(Game *game, EntityHandle id);

Entity *EntityGet(Game *game, EntityHandle id);
EntityHandle EntityGetByEventID(Game *game, CE_u64 eventID);

int EntityQueryFlags(Game *game, EntityHandle id, CE_u64 flags);
void EntitySetFlags(Game *game, EntityHandle id, CE_u64 flags);
void EntityResetFlags(Game *game, EntityHandle id, CE_u64 flags);

void EntityUpdate(Game *game);

#endif
