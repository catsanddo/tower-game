#include "entity.h"
#include "render.h"
#include "game.h"
#include "common.h"

const EntityTemplate entity_table[] = {
    {"Blank",      0, ENTITY_TRIGGER},

    {"Fruit",      0, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE},
    {"Lever",      0, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE},
    {"Door",       0, ENTITY_VISIBLE | ENTITY_SOLID},
    {"Orb",        0, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE},
    {"Orb Placed", 0, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE},

    {"Ghost Girl", 200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"Sword Man",  200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"Bug Man",    200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"Dream Worm", 200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"Skuller",    200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"Mook",       200, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},
    {"The Seer",   175, ENTITY_VISIBLE | ENTITY_SOLID | ENTITY_INTERACTABLE | ENTITY_ANIMATE},

    {"Player",     64, ENTITY_VISIBLE | ENTITY_PLAYER},
};

EntityHandle EntityNew(Game *game, CE_u64 index, Vec2 position, CE_u64 eventID)
{
    if (index >= sizeof(entity_table) / sizeof(entity_table[0])) {
        LogWarning("Abort creating entity: index (%llu) is out of bounds", index);
        return EntityNil();
    }
    EntityTemplate template = entity_table[index];
    
    return EntityConstruct(game, position, template.sprite_name, template.frame_delay,
            eventID, template.flags);
}

EntityHandle EntityConstruct(Game *game, Vec2 position, const char *sprite_name,
        CE_u32 frame_delay, CE_u64 eventID, CE_u64 flags)
{
    EntityPool *pool = game->entity_pool;
    CE_u64 result;

    for (CE_u64 i = 0; i < MAX_ENTITIES; ++i) {
        if ((pool->pool[i].flags & ENTITY_ACTIVE) == 0) {
            result = i;
            break;
        }
    }

    if (result >= MAX_ENTITIES) {
        LogWarning("Failed to allocate entity: no new slots");
        return EntityNil();
    }

    SpriteHandle sprite;
    pool->pool[result].flags = ENTITY_ACTIVE | flags;
    pool->pool[result].eventID = eventID;
    pool->pool[result].momentum = (Vec2) {0};
    pool->pool[result].co = (Coord) {position, {0, 0}};
    pool->pool[result].sprite = SpriteLoad(game, sprite_name, frame_delay);

    if (SpriteIsNil(pool->pool[result].sprite)) {
        LogWarning("Could not load sprite '%s'", sprite_name);
        return EntityNil();
    }

    if (flags & ENTITY_ANIMATE) {
        SpriteStartAnimation(game->renderer, pool->pool[result].sprite);
    }
    
    return (EntityHandle) {result};
}

void EntityFree(Game *game, EntityHandle id)
{
    Entity *entity = EntityGet(game, id);

    if (!entity) {
        return;
    }

    entity->flags = 0;
    SpriteFree(game->renderer, entity->sprite);
}

Entity *EntityGet(Game *game, EntityHandle id)
{
    if (id.handle < 0 || id.handle >= MAX_ENTITIES) {
        return 0;
    }
    
    Entity *result = &game->entity_pool->pool[id.handle];

    if ((result->flags & ENTITY_ACTIVE) == 0) {
        return 0;
    }

    return result;
}

EntityHandle EntityGetByEventID(Game *game, CE_u64 eventID)
{
    EntityPool *pool = game->entity_pool;
    CE_i64 result = -1;
    
    for (CE_u64 i = 0; i < MAX_ENTITIES; ++i) {
        if ((pool->pool[i].flags & ENTITY_ACTIVE) && pool->pool[i].eventID == eventID) {
            result = i;
            break;
        }
    }

    return (EntityHandle) {result};
}

int EntityQueryFlags(Game *game, EntityHandle id, CE_u64 flags)
{
    Entity *entity = EntityGet(game, id);
    if (!entity) {
        return 0;
    }

    return (entity->flags & flags) == flags;
}

void EntitySetFlags(Game *game, EntityHandle id, CE_u64 flags)
{
    Entity *entity = EntityGet(game, id);
    if (!entity) {
        LogWarning("Setting flags of nil entity");
        return;
    }

    entity->flags |= flags;
}

void EntityResetFlags(Game *game, EntityHandle id, CE_u64 flags)
{
    Entity *entity = EntityGet(game, id);
    if (!entity) {
        LogWarning("Resetting flags of nil entity");
        return;
    }

    entity->flags &= ~flags;
    // safeguard against erroneous deactivation
    // entity->flags |= ENTITY_ACTIVE;
}

void EntityUpdate(Game *game)
{
    Entity *player = 0;
    EntityForEach(entity) {
        // draw entities
        if (EntityQueryFlags(game, entity, ENTITY_VISIBLE)) {
            Entity *e = EntityGet(game, entity);
            SpriteDraw(game->renderer, e->sprite, e->co);
        }
        if (EntityQueryFlags(game, entity, ENTITY_PLAYER)) {
            player = EntityGet(game, entity);
        }
    }

    if (!player) {
        return;
    }
    int trigger_flag = 0;
    if (!game->triggered && player->momentum.x == 0 && player->momentum.y == 0) {
        EntityForEach(handle) {
            if (EntityQueryFlags(game, handle, ENTITY_TRIGGER)) {
                Entity *entity = EntityGet(game, handle);
                if (entity->co.position.x == player->co.position.x &&
                        entity->co.position.y == player->co.position.y) {
                    game->triggered = 1;
                    trigger_flag = 1;
                    if (!ScriptStartEvent(game, entity->eventID)) {
                        // TODO: find a better way to request quitting
                        game->input.quit = 1;
                    }
                }
            }
        }
    }
    if (!game->input_lock && game->input.action_pressed) {
        // NOTE: hack
        Vec2 target = player->co.position;
        switch (player->direction) {
            case DOWN_ANIMATION: {
                target.y += 1;
            } break;
            case RIGHT_ANIMATION: {
                target.x += 1;
            } break;
            case LEFT_ANIMATION: {
                target.x -= 1;
            } break;
            case UP_ANIMATION: {
                target.y -= 1;
            } break;
        }
        EntityForEach(handle) {
            if (EntityQueryFlags(game, handle, ENTITY_INTERACTABLE)) {
                Entity *entity = EntityGet(game, handle);
                if (entity->co.position.x == target.x &&
                        entity->co.position.y == target.y) {
                    if (!ScriptStartEvent(game, entity->eventID)) {
                        // TODO: find a better way to request quitting
                        // Wait, what's happening here?
                        // Is this supposed to be a graceful crash??
                        // Where's the logging???
                        game->input.quit = 1;
                    }
                }
            }
        }
    }
    // TODO: turn at each new keystroke with key release events
    // okay so this means that you should be able to hold the left direction
    // then press up while still holding left and go up
    // basically each direction can be interrupted by another
    // it might get a bit complex but I think the controls will feel pretty good this way
    // also maybe watch pokemon/Saga gameplay for movement tips
    if (!game->input_lock && !trigger_flag) {
        Vec2 direction = {
            game->input.right - game->input.left,
            game->input.down - game->input.up,
        };
        if (direction.y) {
            direction.x = 0;
        }
        Vec2 new_position = player->co.position;
        new_position.x += direction.x;
        new_position.y += direction.y;
        if (direction.x || direction.y) {
            int anim = direction.x > 0 ? RIGHT_ANIMATION : direction.x < 0 ? 
                    LEFT_ANIMATION : direction.y > 0 ? DOWN_ANIMATION : UP_ANIMATION;
            SpriteSetAnimation(game->renderer, player->sprite, anim);
            player->direction = anim;
            if (!GetCollision(game, new_position)) {
                player->momentum = direction;
                game->input_lock = 1;
                game->triggered = 0;
                SpriteStartAnimation(game->renderer, player->sprite);
            }
        } else {
            // TODO: debug why this doesn't fire when standing still
            SpriteResetAnimation(game->renderer, player->sprite);
            SpriteStopAnimation(game->renderer, player->sprite);
        }
    }

    SetCamera(game->renderer, player->co);
    if (game->input.cancel) {
    player->co.offset.x += player->momentum.x * ENTITY_SPEED * 4;
        player->co.offset.y += player->momentum.y * ENTITY_SPEED * 4;
    } else {
        player->co.offset.x += player->momentum.x * ENTITY_SPEED;
        player->co.offset.y += player->momentum.y * ENTITY_SPEED;
    }
    if (Abs(player->co.offset.x) >= TILE || Abs(player->co.offset.y) >= TILE) {
        player->co.position.x += player->momentum.x;
        player->co.position.y += player->momentum.y;
        player->momentum = (Vec2) {0};
        player->co.offset = (Vec2) {0};
        game->input_lock = 0;
    }
}
