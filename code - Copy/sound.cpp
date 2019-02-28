#include "basecode/os.hpp"

#include "sound.hpp"

#if OS != OS_IOS

void init_sound(SoundInfo *sound){
    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0){
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    char path[MAX_PATH_LENGTH];
    
    get_game_file_path("Sound/sound_walk_0.wav", path);
    sound->sounds[SOUND_WALK_0] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_WALK_0] == NULL){
        printf( "Failed to load 'sound_walk_0'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_walk_1.wav", path);
    sound->sounds[SOUND_WALK_1] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_WALK_1] == NULL){
        printf( "Failed to load 'sound_walk_1'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_jump.wav", path);
    sound->sounds[SOUND_JUMP] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_JUMP] == NULL){
        printf( "Failed to load 'sound_jump'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_death.wav", path);
    sound->sounds[SOUND_DEATH] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_DEATH] == NULL){
        printf( "Failed to load 'sound_death'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_win.wav", path);
    sound->sounds[SOUND_WIN] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_WIN] == NULL){
        printf( "Failed to load 'sound_win'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_collision.wav", path);
    sound->sounds[SOUND_COLLISION] = Mix_LoadWAV(path);
    if(sound->sounds[SOUND_COLLISION] == NULL){
        printf( "Failed to load 'sound_win'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
}

void cleanup_sound(SoundInfo *sound){
    for(int i=0; i<sounds_size; i++)
        Mix_FreeChunk(sound->sounds[i]);
    Mix_Quit();
}

void play_sound(SoundInfo *sound, u8 sound_index){
    if(!sound->on)
        return;
    //Mix_PlayChannel(-1, sound->sounds[sound_index], 0);
}

#else

void init_sound(){
    
}
void cleanup_sound(){
    
}
void play_step(){
    
}
void play_sound(u8 sound_index){
}

#endif
