#include "sound.h"
#include "log.h"
#include "common.h"

const char *sfx_names[] = {
    "blip",
    "plib",
    "quake",
};

const char *track_names[] = {
    "1 Title-Tower Top",
    "2 Dreaming",
    "3 Forest Theme",
    "4 Cave Theme",
    "5 Swamp Theme",
    "6 Beach Theme",
    "7 Ruins Theme",
    "8 Town Theme",
    "9 Tower Entrance",
    "10 Tower Theme",
    "11 Credits",
};

int SoundEngineInit(CE_Arena *arena, SoundEngine *engine)
{
    ma_result result = ma_engine_init(0, &engine->engine);
    if (result != MA_SUCCESS) {
        AlertError("Audio Error", "Could not initialize audio engine!");
        return 0;
    }

    result = ma_sound_group_init(&engine->engine, 0, 0, &engine->sfx_group);
    if (result != MA_SUCCESS) {
        AlertError("Audio Error", "Could not initialize sfx audio group");
        return 0;
    }
    result = ma_sound_group_init(&engine->engine, 0, 0, &engine->track_group);
    if (result != MA_SUCCESS) {
        AlertError("Audio Error", "Could not initialize track audio group");
        return 0;
    }

    engine->sfxs = CE_ArenaPush(arena, sizeof(ma_sound) * StaticArraySize(sfx_names));
    engine->is_track = 0;

    ma_fence fence;
    result = ma_fence_init(&fence);
    if (result != MA_SUCCESS) {
        AlertError("Audio Error", "Fence error");
        return 0;
    }
    
    // TODO: push a watermark on the arena for tmp data
    for (CE_u64 i = 0; i < StaticArraySize(sfx_names); ++i) {
        CE_u64 size = snprintf(0, 0, "res/sfx/%s.wav", sfx_names[i]);
        char *path = CE_ArenaPush(arena, size + 1);
        snprintf(path, size+1, "res/sfx/%s.wav", sfx_names[i]);

        result = ma_sound_init_from_file(&engine->engine, path, MA_SOUND_FLAG_ASYNC,
                &engine->sfx_group, &fence, &engine->sfxs[i]);
        if (result != MA_SUCCESS) {
            AlertError("Resource Error", "Unable to initialize sfx '%s'", path);
            return 0;
        }
    }

    // Wait for sounds to asynchronously load
    ma_fence_wait(&fence);

    return 1;
}

void SoundEngineDeinit(SoundEngine *engine)
{
    for (CE_u64 i = 0; i < StaticArraySize(sfx_names); ++i) {
        ma_sound_uninit(&engine->sfxs[i]);
    }

    ma_sound_group_uninit(&engine->sfx_group);
    ma_sound_group_uninit(&engine->track_group);

    ma_engine_uninit(&engine->engine);

    *engine = (SoundEngine) {0};
}

void PlaySfx(SoundEngine *engine, CE_i32 index)
{
    if (index < 0 || index > StaticArraySize(sfx_names)) {
        LogWarning("Sfx index out of bounds (%d)", index);
        return;
    }

    if (index == 0) {
        ma_sound_stop((ma_sound *) &engine->sfx_group);
        return;
    }

    index -= 1;
    ma_sound_seek_to_pcm_frame(&engine->sfxs[index], 0);
    ma_sound_start(&engine->sfxs[index]);
}

void PlayTrack(SoundEngine *engine, CE_Arena *arena, CE_i32 index)
{
    if (index < 0 || index > StaticArraySize(track_names)) {
        LogWarning("Sfx index out of bounds (%d)", index);
        return;
    }
    
    if (engine->is_track) {
        ma_sound_stop(&engine->current_track);
        ma_sound_uninit(&engine->current_track);
    }
    if (index == 0) {
        engine->is_track = 0;
        return;
    }

    index -= 1;
    CE_u64 size = snprintf(0, 0, "res/tracks/%s.mp3", track_names[index]);
    char *path = CE_ArenaPush(arena, size + 1);
    snprintf(path, size+1, "res/tracks/%s.mp3", track_names[index]);
    
    ma_result result = ma_sound_init_from_file(&engine->engine, path, 0,
            &engine->track_group, 0, &engine->current_track);
    if (result != MA_SUCCESS) {
        LogWarning("Could not load track '%s'", path);
        return;
    }
    ma_sound_set_looping(&engine->current_track, 1);
    ma_sound_start(&engine->current_track);
    
    engine->is_track = 1;
}

