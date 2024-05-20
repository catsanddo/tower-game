#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

extern const char *sfx_names[];
extern const char *track_names[];

typedef struct SoundEngine SoundEngine;
struct SoundEngine {
    bool is_track;
    ma_engine engine;
    ma_sound *sfxs;
    ma_sound_group sfx_group;
    ma_sound_group track_group;
    ma_sound current_track;
};

int SoundEngineInit(CE_Arena *arena, SoundEngine *engine);
void SoundEngineDeinit(SoundEngine *engine);
void PlaySfx(SoundEngine *engine, CE_i32 index);
void PlayTrack(SoundEngine *engine, CE_Arena *arena, CE_i32 index);

#endif
