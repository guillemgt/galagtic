#include "include/misc_tools.hpp"
#include "sound.hpp"

#if OS != OS_IOS && 0

const float global_volume = 0.2f*MIX_MAX_VOLUME;

void init_sound(SoundInfo *sound){
    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0){
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    char path[MAX_PATH_LENGTH];
    
#define LOAD_SOUND_FILE(id, file, volume) { \
        get_game_file_path(file, path); \
        sound->sounds[id] = Mix_LoadWAV(path); \
        if(sound->sounds[id] == NULL){ \
            printf( "Failed to load '" file "'! SDL_mixer Error: %s\n", Mix_GetError() ); \
            return; \
        } \
        Mix_VolumeChunk(sound->sounds[id], (int)(volume*global_volume)); \
    } \
    
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

#else

void init_sound(SoundInfo *sound){
}
void cleanup_sound(){
    
}
void play_step(){
    
}

void play_sound(SoundInfo *sound, u8 sound_index){
}

#endif
