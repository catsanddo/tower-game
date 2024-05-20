#include "cutscene.h"
#include "game.h"
#include "entity.h"
#include "common.h"

void RegisterCutscene(Game *game, CutSceneFn function, void *data)
{
    game->cutscene = function;
    game->cutscene_data = data;
}

int cs_Move(Game *game)
{
    int will_continue = 0;
    EntityHandle blocking_id = EntityGetByEventID(game, 400);

    game->input_lock = 1;

    if (!EntityIsNil(blocking_id)) {
        Entity *blocking = EntityGet(game, blocking_id);
        if (blocking->co.offset.y < TILE) {
            blocking->co.offset.y += game->clock.frames & 1;
            will_continue = 1;
            goto end;
        } else if (Abs(blocking->co.offset.x) < TILE) {
            blocking->co.offset.x -= game->clock.frames & 1;
            will_continue = 1;
            goto end;
        }

        blocking->co.offset = (Vec2) {0};
        blocking->co.position.x -= 1;
        blocking->co.position.y += 1;
    }
    
end:
    EntityUpdate(game);
    game->input_lock = 0;

    return will_continue;
}

const CutSceneFn cutscene_table[] = {
    cs_Move,
};
