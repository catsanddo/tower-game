#include "script.h"
#include "game.h"
#include "map.h"
#include "render.h"
#include "sound.h"
#include "log.h"
#include "common.h"

#include <stdio.h>

static CE_u64 ScriptGetArg(Script *script, int argument)
{
    CE_u64 result = 0;
    if (script->offset + 4 > script->buffer.len) {
        LogWarning("Unexpected EOF when parsing script argument");
        return 0;
    }

    argument *= 5;
    argument += 4;
    
    result += script->buffer.str[script->offset+argument] - '0';
    result *= 10;
    result += script->buffer.str[script->offset+argument+1] - '0';
    result *= 10;
    result += script->buffer.str[script->offset+argument+2] - '0';
    result *= 10;
    result += script->buffer.str[script->offset+argument+3] - '0';

    return result;
}

static void ScriptSeekNext(Script *script)
{
    while (script->offset < script->buffer.len &&
            script->buffer.str[script->offset] != ':') {
        script->offset += 1;
    }
    script->offset += 4;
}

int ScriptInit(Game *game, Script *script)
{
    *script = (Script) {0};
    script->buffer.str = CE_ArenaPush(game->global, SCRIPT_BUFFER_SIZE);

    return 1;
}

int ScriptLoad(Game *game, const char *path)
{
    Script *script = game->script;
    FILE *file = fopen(path, "r");
    CE_u64 length;

    if (!file) {
        AlertError("Resource Error", "Could not open script '%s'", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    length = ftell(file);

    if (script->buffer.len + length > SCRIPT_BUFFER_SIZE) {
        float size = script->buffer.len + length;
        size /= 1024; // Kibibytes
        AlertError("Resource Error", "Script '%s' exceeded buffer limits (%.2f)", path, size);
        fclose(file);
        return 0;
    }
    
    rewind(file);
    fread(script->buffer.str + script->buffer.len, 1, length, file);
    script->buffer.len += length;
    script->active = 0;

    fclose(file);

    return 1;
}

int ScriptStartEvent(Game *game, CE_u64 eventID)
{
    Script *script = game->script;

    String reader = script->buffer;
    int new_line = 1;
    CE_u64 id = 0;
    CE_u64 offset = 0;
    while (reader.len >= 5) {
        if (reader.str[0] == '#' && new_line) {
            id = 0;
            id += reader.str[1] - '0';
            id *= 10;
            id += reader.str[2] - '0';
            id *= 10;
            id += reader.str[3] - '0';
            id *= 10;
            id += reader.str[4] - '0';

            if (id == eventID) {
                break;
            }
        } else if (reader.str[0] == '\n') {
            new_line = 1;
        } else {
            new_line = 0;
        }
        
        reader.str += 1;
        reader.len -= 1;
        offset += 1;
    }

    // could not find id in script; fatal error
    if (id != eventID) {
        AlertError("Script Error", "Failed to find event '%llu' in script", eventID);
        return 0;
    }

    // Seek the next line
    while (offset < script->buffer.len && script->buffer.str[offset] != '\n') {
        offset += 1;
    }
    offset += 1;

    script->event = eventID;
    script->active = 1;
    script->offset = offset;
    script->in_text = 0;
    script->timer = 0;
    script->prompt_state = 0;

    return 1;
}

int ScriptInterpret(Game *game) {
    Script *script = game->script;
    int tbox_open = TextBoxIsOpen(game->renderer);

    if (script->offset >= script->buffer.len) {
        return SCRIPT_ERROR;
    }

    // script wait conditions
    // cutscene is running
    // curtain is moving
    if (game->cutscene || CurtainInMotion(game->renderer)) {
        return SCRIPT_RUNNING;
    }
    if (script->timer > 0) {
        script->timer -= 1000 / MAX_FPS;
        script->timer = Max(script->timer, 0);
        return SCRIPT_RUNNING;
    }

    // prompt handling
    if (script->prompt_state != PROMPT_NONE) {
        if (game->input.up_pressed || game->input.down_pressed) {
            if (script->prompt_state == PROMPT_YES) {
                script->prompt_state = PROMPT_NO;
            } else {
                script->prompt_state = PROMPT_YES;
            }
        } else if (game->input.action_pressed) {
            if (script->prompt_state == PROMPT_NO) {
                ScriptSeekNext(script);
            }
            script->prompt_state = PROMPT_NONE;
        }
        PromptDraw(game->renderer, script->prompt_state);
        return SCRIPT_RUNNING;
    }

    if ((script->in_text || script->buffer.str[script->offset] == '"')
            && tbox_open) {
        // TODO: remove slower text hack
        if (TextBoxIsScrolling(game->renderer) || game->clock.frames & 1) {
            return SCRIPT_RUNNING;
        }
        if (script->buffer.str[script->offset] == '"' && !script->in_text) {
            script->in_text = 1;
            script->offset += 1;
            return SCRIPT_RUNNING;
        } else if (script->buffer.str[script->offset] == '"') {
            script->in_text = 0;
            script->offset += 1;
            return SCRIPT_RUNNING;
        }

        // String text;
        // text.str = script->buffer.str + script->offset;
        // text.len = 1;
        // if (text.str[0] == '\\') {
        //     script->offset += 1;
        //     switch (script->buffer.str[script->offset]) {
        //         case '\\':
        //             text.str[0] = '\\';
        //         break;
        //         case '"':
        //             text.str[0] = '"';
        //         break;
        //         case 'n':
        //             text.str[0] = '\n';
        //         break;
        //     }
        // }
        
        // if (text.str[0] == '\n') {
        //     TextBoxNewLine(game->renderer);
        // } else {
        //     TextBoxAppend(game->renderer, text);
        // }
        int loops = 1;
        if (game->input.cancel) {
            loops = 4;
        }
        for (int i = 0; i < loops; ++i) {
            if (script->buffer.str[script->offset] == '"') {
                break;
            }
            char text = script->buffer.str[script->offset];
            if (text == '\\') {
                script->offset += 1;
                switch (script->buffer.str[script->offset]) {
                    case '\\':
                        text = '\\';
                    break;
                    case '"':
                        text = '"';
                    break;
                    case 'n':
                        text = '\n';
                    break;
                }
            }
        
            if (text == '\n') {
                TextBoxNewLine(game->renderer);
            } else {
                TextBoxAppend(game->renderer, (String) {&text, 1});
            }
            script->offset += 1;
        }
        return SCRIPT_RUNNING;
    } else if (script->buffer.str[script->offset] != ':') {
        while (script->offset < script->buffer.len) {
            if (script->buffer.str[script->offset] == ':') {
                break;
            } else if (script->buffer.str[script->offset] == '"') {
                return SCRIPT_RUNNING;
            } else if (script->buffer.str[script->offset] == ';') {
                while (script->offset < script->buffer.len &&
                        script->buffer.str[script->offset] != '\n') {
                    script->offset += 1;
                }
                script->offset += 1;
                return SCRIPT_RUNNING;
            }
            script->offset += 1;
        }
        if (script->offset + 4 > script->buffer.len) {
            return SCRIPT_ERROR;
        }
    }

    char *cmd_name = script->buffer.str + script->offset;
    int result = SCRIPT_RUNNING;
    if (cmd_name[1] == 'E' && cmd_name[2] == 'N' && cmd_name[3] == 'D') {
        game->input_lock = 0;
        script->active = 0;
        result = SCRIPT_DONE;
    } else if (cmd_name[1] == 'T' && cmd_name[2] == 'X' && cmd_name[3] == 'T') {
        TextBoxOpenTop(game->renderer);
    } else if (cmd_name[1] == 'T' && cmd_name[2] == 'X' && cmd_name[3] == 'B') {
        TextBoxOpenBottom(game->renderer);
    } else if (cmd_name[1] == 'T' && cmd_name[2] == 'C' && cmd_name[3] == 'R') {
        TextBoxClear(game->renderer);
    } else if (cmd_name[1] == 'T' && cmd_name[2] == 'C' && cmd_name[3] == 'L') {
        TextBoxClose(game->renderer);
    } else if (cmd_name[1] == 'A' && cmd_name[2] == 'N' && cmd_name[3] == 'I') {
        RendererEnableAnimations(game->renderer);
    } else if (cmd_name[1] == 'N' && cmd_name[2] == 'A' && cmd_name[3] == 'N') {
        RendererDisableAnimations(game->renderer);
    } else if (cmd_name[1] == 'F' && cmd_name[2] == 'D' && cmd_name[3] == 'O') {
        CurtainFall(game->renderer);
    } else if (cmd_name[1] == 'F' && cmd_name[2] == 'D' && cmd_name[3] == 'I') {
        CurtainRaise(game->renderer);
    } else if (cmd_name[1] == 'P' && cmd_name[2] == 'A' && cmd_name[3] == 'U') {
        if (!game->input.action_pressed) {
            return result;
        }
    } else if (cmd_name[1] == 'L' && cmd_name[2] == 'C' && cmd_name[3] == 'K') {
        game->input_lock = 1;
    } else if (cmd_name[1] == 'W' && cmd_name[2] == 'A' && cmd_name[3] == 'T') {
        CE_u64 timeout = ScriptGetArg(script, 0);
        script->timer = timeout;
        script->offset += 4;
    } else if (cmd_name[1] == 'F' && cmd_name[2] == 'L' && cmd_name[3] == 'S') {
        CE_u64 flag = ScriptGetArg(script, 0);
        GlobalFlagSet(game, flag);
        script->offset += 4;
    } else if (cmd_name[1] == 'F' && cmd_name[2] == 'L' && cmd_name[3] == 'R') {
        CE_u64 flag = ScriptGetArg(script, 0);
        GlobalFlagReset(game, flag);
        script->offset += 4;
    } else if (cmd_name[1] == 'F' && cmd_name[2] == 'L' && cmd_name[3] == 'J') {
        CE_u64 flag = ScriptGetArg(script, 0);
        if (!GlobalFlagQuery(game, flag)) {
            ScriptSeekNext(script);
            ScriptSeekNext(script);
            script->offset -= 4;
        } else {
            script->offset += 4;
        }
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'F' && cmd_name[3] == 'S') {
        CE_u64 eventID = ScriptGetArg(script, 0);
        CE_u64 flags = ScriptGetArg(script, 1);
        EntityHandle entity = EntityGetByEventID(game, eventID);
        EntitySetFlags(game, entity, flags);
        script->offset += 9;
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'F' && cmd_name[3] == 'R') {
        CE_u64 eventID = ScriptGetArg(script, 0);
        CE_u64 flags = ScriptGetArg(script, 1);
        EntityHandle entity = EntityGetByEventID(game, eventID);
        EntityResetFlags(game, entity, flags);
        script->offset += 9;
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'F' && cmd_name[3] == 'J') {
        CE_u64 eventID = ScriptGetArg(script, 0);
        CE_u64 flags = ScriptGetArg(script, 1);
        EntityHandle entity = EntityGetByEventID(game, eventID);
        if (!EntityQueryFlags(game, entity, flags)) {
            ScriptSeekNext(script);
            ScriptSeekNext(script);
            script->offset -= 4;
        } else {
            script->offset += 9;
        }
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'M' && cmd_name[3] == 'V') {
        CE_u64 eventID = ScriptGetArg(script, 0);
        CE_u64 dir = ScriptGetArg(script, 1);
        CE_u64 amount = ScriptGetArg(script, 2);

        Entity *entity = EntityGet(game, EntityGetByEventID(game, eventID));
        switch (dir) {
            case DOWN_ANIMATION:
                entity->co.position.y += amount;
                break;
            case RIGHT_ANIMATION:
                entity->co.position.x += amount;
                break;
            case LEFT_ANIMATION:
                entity->co.position.x -= amount;
                break;
            case UP_ANIMATION:
                entity->co.position.y -= amount;
                break;
        }
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'A' && cmd_name[3] == 'N') {
        CE_u64 eventID = ScriptGetArg(script, 0);
        CE_u64 animation = ScriptGetArg(script, 1);
        Entity *entity = EntityGet(game, EntityGetByEventID(game, eventID));
        // TODO: report error?
        if (entity) {
            SpriteSetAnimation(game->renderer, entity->sprite, animation);
        }
    } else if (cmd_name[1] == 'S' && cmd_name[2] == 'K' && cmd_name[3] == 'P') {
        CE_u64 n = ScriptGetArg(script, 0);
        ScriptSeekNext(script);
        for (CE_u64 i = 0; i < n; ++i) {
            ScriptSeekNext(script);
        }
        script->offset -= 4;
    } else if (cmd_name[1] == 'E' && cmd_name[2] == 'V' && cmd_name[3] == 'E') {
        CE_u64 event = ScriptGetArg(script, 0);
        ScriptStartEvent(game, event);
        return SCRIPT_RUNNING;
    } else if (cmd_name[1] == 'P' && cmd_name[2] == 'Y' && cmd_name[3] == 'S') {
        script->prompt_state = PROMPT_YES;
    } else if (cmd_name[1] == 'P' && cmd_name[2] == 'N' && cmd_name[3] == 'O') {
        script->prompt_state = PROMPT_NO;
    } else if (cmd_name[1] == 'S' && cmd_name[2] == 'F' && cmd_name[3] == 'X') {
        CE_u64 sfx_id = ScriptGetArg(script, 0);
        PlaySfx(game->sengine, sfx_id);
    } else if (cmd_name[1] == 'T' && cmd_name[2] == 'R' && cmd_name[3] == 'K') {
        CE_u64 track_id = ScriptGetArg(script, 0);
        PlayTrack(game->sengine, game->frame_arena, track_id);
    } else if (cmd_name[1] == 'M' && cmd_name[2] == 'A' && cmd_name[3] == 'P') {
        CE_u64 map_index = ScriptGetArg(script, 0);
        CE_u64 x = ScriptGetArg(script, 1);
        CE_u64 y = ScriptGetArg(script, 2);
        CE_u64 dir = ScriptGetArg(script, 3);

        // TODO: handle error case
        if (map_index < StaticArraySize(map_names)) {
            MapUnload(game);
            if (!MapLoad(game, map_names[map_index])) {
                game->input.quit = 1;
                return SCRIPT_DONE;
            }
            Vec2 pos = {x, y};
            Entity *player = EntityGet(game, EntityNew(game, ET_PLAYER, pos, 0));
            SpriteSetAnimation(game->renderer, player->sprite, dir);

            ScriptStartEvent(game, 20);
            return SCRIPT_RUNNING;
        }
        script->offset += 14;
    } else if (cmd_name[1] == 'C' && cmd_name[2] == 'U' && cmd_name[3] == 'T') {
        CE_u64 cutscene_index = ScriptGetArg(script, 0);
        // TODO: handle error case
        if (cutscene_index < StaticArraySize(cutscene_table))  {
            RegisterCutscene(game, cutscene_table[cutscene_index], 0);
        }
        script->offset += 4;
    } else {
        result = SCRIPT_ERROR;
    }

    script->offset += 4;

    return result;
}
