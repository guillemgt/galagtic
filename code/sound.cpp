#include "include/misc_tools.hpp"
#include "sound.hpp"

#if SOUND_ENGINE == SOUND_ENGINE_JS


extern "C" {
    extern void loadAudio(int slot, const char *path, float volume);
    extern void loadAudioLoop(int slot, const char *path, float volume);
    extern void playAudio(int slot);
    extern void stopAudio(int slot);
    extern void doEndingAudioEffect();
}

const float global_volume = 0.2f;

void init_sound(SoundInfo *sound){
#define LOAD_SOUND_FILE(slot, path, vol) loadAudio(slot, path, global_volume*vol)
#define LOAD_SOUND_LOOP_FILE(slot, path, vol) loadAudioLoop(slot, path, global_volume*vol)
    
    LOAD_SOUND_FILE(SOUND_WALK_0, "Sound/sound_walk_0.wav", 0.3f);
    LOAD_SOUND_FILE(SOUND_WALK_1, "Sound/sound_walk_1.wav", 0.3f);
    LOAD_SOUND_FILE(SOUND_JUMP,   "Sound/sound_walk_0.wav", 0.4f);
    LOAD_SOUND_FILE(SOUND_DEATH,  "Sound/sound_death.wav",  1.f);
    LOAD_SOUND_FILE(SOUND_WIN,    "Sound/sound_win.wav",    0.7f);
    LOAD_SOUND_FILE(SOUND_SLIDE,  "Sound/sound_slide.wav",  0.3f);
    LOAD_SOUND_FILE(SOUND_SPIKES, "Sound/sound_spikes.wav", 0.6f);
    LOAD_SOUND_FILE(SOUND_TICK_0, "Sound/tick0.wav",        1.3f);
    LOAD_SOUND_FILE(SOUND_TICK_1, "Sound/tick1.wav",        1.3f);
    LOAD_SOUND_LOOP_FILE(SOUND_MACHINE, "Sound/explosion.wav", 1.3f);
    LOAD_SOUND_FILE(SOUND_HIGHPITCH, "Sound/ending.wav", 1.f / global_volume);
}

void cleanup_sound(SoundInfo *sound){
}

void play_sound(SoundInfo *sound, u8 sound_index){
    if(!sound->on)
        return;
    playAudio(sound_index);
}

void stop_sound(SoundInfo *sound, u8 sound_index){
    if(!sound->on)
        return;
    stopAudio(sound_index);
}






#elif SOUND_ENGINE == SOUND_ENGINE_SDL

const float global_volume = 0.2f*MIX_MAX_VOLUME;

void init_sound(SoundInfo *sound){
    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0){
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    char path[MAX_PATH_LENGTH];
    
#define LOAD_SOUND_FILE(id, file, volume) { \
ggtp_program_file_path(file, path); \
sound->sounds[id] = Mix_LoadWAV(path); \
if(sound->sounds[id] == NULL){ \
printf( "Failed to load '" file "'! SDL_mixer Error: %s\n", Mix_GetError() ); \
return; \
} \
} \
//Mix_VolumeChunk(sound->sounds[id], (int)(volume*global_volume)); \
    
    LOAD_SOUND_FILE(SOUND_WALK_0, "Sound/sound_walk_0.wav", 0.3f);
    LOAD_SOUND_FILE(SOUND_WALK_1, "Sound/sound_walk_1.wav", 0.3f);
    LOAD_SOUND_FILE(SOUND_JUMP,   "Sound/sound_walk_0.wav", 0.4f);LOAD_SOUND_FILE(SOUND_DEATH,  "Sound/sound_death.wav",  1.f);
    LOAD_SOUND_FILE(SOUND_WIN,    "Sound/sound_win.wav",    0.7f);
    LOAD_SOUND_FILE(SOUND_SLIDE,  "Sound/sound_slide.wav",  0.3f);
    LOAD_SOUND_FILE(SOUND_SPIKES, "Sound/sound_spikes.wav", 0.6f);
    LOAD_SOUND_FILE(SOUND_TICK_0, "Sound/tick0.wav",        1.3f);
    LOAD_SOUND_FILE(SOUND_TICK_1, "Sound/tick1.wav",        1.3f);
}

void cleanup_sound(SoundInfo *sound){
    for(int i=0; i<sounds_size; i++)
        Mix_FreeChunk(sound->sounds[i]);
    Mix_Quit();
}

void play_sound(SoundInfo *sound, u8 sound_index){
    if(!sound->on)
        return;
    Mix_PlayChannel(-1, sound->sounds[sound_index], 0);
}




#elif SOUND_ENGINE == SOUND_ENGINE_CUTE




const float global_volume = 0.2f;

void load_sound_file(SoundInfo *sound, u8 id, const char *file, float volume){
    char path[MAX_PATH_LENGTH];
    ggtp_program_file_path(file, path);
    sound->audios[id] = cs_load_wav(path);
    sound->sounds[id] = cs_make_def(&sound->audios[id]);
    sound->sounds[id].volume_left = global_volume*volume;
    sound->sounds[id].volume_right = global_volume*volume;
}

void init_sound(SoundInfo *sound){
    sound->ctx = cs_make_context(ggt_platform_globals.hWnd, 48000, 4092 * 2, 128, NULL);
    if(sound->ctx == NULL){
        printf("Couldn't initialize cute_sound context!\n");
    }
    
    load_sound_file(sound, SOUND_WALK_0,    "Sound/sound_walk_0.wav", 0.3f);
    load_sound_file(sound, SOUND_WALK_1,    "Sound/sound_walk_1.wav", 0.3f);
    load_sound_file(sound, SOUND_JUMP,      "Sound/sound_walk_0.wav", 0.4f);
    load_sound_file(sound, SOUND_DEATH,     "Sound/sound_death.wav",  1.f );
    load_sound_file(sound, SOUND_WIN,       "Sound/sound_win.wav",    0.7f);
    load_sound_file(sound, SOUND_SLIDE,     "Sound/sound_slide.wav",  0.3f);
    load_sound_file(sound, SOUND_SPIKES,    "Sound/sound_spikes.wav", 0.6f);
    load_sound_file(sound, SOUND_TICK_0,    "Sound/tick0.wav",        1.3f);
    load_sound_file(sound, SOUND_TICK_1,    "Sound/tick1.wav",        1.3f);
    load_sound_file(sound, SOUND_MACHINE,   "Sound/explosion.wav",    1.3f);
    sound->sounds[SOUND_MACHINE].looped = 1;
    load_sound_file(sound, SOUND_HIGHPITCH, "Sound/ending.wav",       1.f/global_volume);
}

void cleanup_sound(SoundInfo *sound){
    for(int i=0; i<sounds_size; i++)
        cs_free_sound(&sound->audios[i]);
}

void play_sound(SoundInfo *sound, u8 sound_index){
    if(!sound->on)
        return;
    cs_play_sound(sound->ctx, sound->sounds[sound_index]);
}
void stop_sound(SoundInfo *sound_info, u8 sound_index){
    cs_context_t *ctx = sound_info->ctx;
    
	cs_lock(ctx);
	cs_playing_sound_t* sound = ctx->playing;
	while(sound){
		// let cs_mix() remove the sound
        if(sound->loaded_sound == &sound_info->audios[sound_index])
            sound->active = 0;
		sound = sound->next;
	}
	cs_unlock(ctx);
}

void process_sound(GameState *game_state){
    SoundInfo *sound = &game_state->sound;
    cs_mix(sound->ctx);
}


#else

void init_sound(SoundInfo *sound){
}
void cleanup_sound(){
    
}

void play_sound(SoundInfo *sound, u8 sound_index){
}
void stop_sound(SoundInfo *sound, u8 sound_index){
}
void doEndingAudioEffect(){
}

#endif
