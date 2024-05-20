#include <stdio.h>
#include <cose/cose.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#define LOG_FILE "game.log"

#include "config.h"
#include "common.h"
#include "game.h"
#include "log.h"
#include "render.h"
#include "map.h"
#include "entity.h"
#include "cutscene.h"
#include "script.h"
#include "sound.h"

#include "game.c"
#include "log.c"
#include "render.c"
#include "map.c"
#include "entity.c"
#include "cutscene.c"
#include "script.c"
#include "sound.c"

int main(int argc, char **argv)
{
    Game game;
    if (!GameInit(&game)) {
        return 1;
    }

    // Load first map
    MapLoad(&game, "Forest");
    
    // Create player
    Vec2 pos = {0};
    pos = (Vec2) {66, 107};
    EntityHandle playerID = EntityNew(&game, ET_PLAYER, pos, 0);

    // Close curtain and start first event
    game.renderer->curtain.target_height = HEIGHT;
    game.renderer->curtain.height = HEIGHT;
    ScriptStartEvent(&game, 200);

    int running = 1;
    while (running) {
        GrabInput(&game);
        running = !game.input.quit;

        // if (game.input.action_pressed && !game.input_lock) {
        //     RegisterCutscene(&game, cs_Move, 0);
        // }

        if (game.script->active) {
            int result = ScriptInterpret(&game);
            // TODO: better script error reporting
            if (result == SCRIPT_ERROR) {
                AlertError("Script Error", "Fatal script error");
                running = 0;
            } else if (result == SCRIPT_DONE) {
                game.script->active = 0;
            }
        }

        if (game.cutscene) {
            int will_continue = game.cutscene(&game);
            if (!will_continue) {
                game.cutscene = 0;
                game.cutscene_data = 0;
            }
        } else {
            EntityUpdate(&game);
        }

        RendererDraw(game.renderer);

        Tick(&game);
    }

    GameDeinit(&game);
    
    return 0;
}
