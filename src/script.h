#ifndef SCRIPT_H
#define SCRIPT_H

#include "common.h"

#define SCRIPT_BUFFER_SIZE (10L << 10)

enum PromptState {
    PROMPT_NONE,
    PROMPT_NO,
    PROMPT_YES,
};

enum ScriptResult {
    SCRIPT_RUNNING,
    SCRIPT_ERROR,
    SCRIPT_DONE,
};

typedef struct Script Script;
struct Script {
    bool active;
    bool in_text;
    // bool visited;
    int prompt_state;
    CE_u64 offset;
    CE_u64 event;
    CE_i64 timer;
    String buffer;
};

int ScriptInit(Game *game, Script *script);
int ScriptLoad(Game *game, const char *path);
int ScriptStartEvent(Game *game, CE_u64 eventID);
int ScriptInterpret(Game *game);

#endif
