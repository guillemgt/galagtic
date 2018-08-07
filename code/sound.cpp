#include "basecode/os.hpp"

#include "sound.hpp"

bool sound_on = true;

#if OS != OS_IOS

#include "basecode/sdl2.hpp"
#if OS != OS_WASM
#include <SDL2_mixer/SDL_mixer.h>
#else
#include <SDL/SDL_mixer.h>
#endif

const int sounds_size = 6;
static Mix_Chunk *sounds[sounds_size];
void init_sound(){
    if(Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0){
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    char path[MAX_PATH_LENGTH];
    
    get_game_file_path("Sound/sound_walk_0.wav", path);
    sounds[SOUND_WALK_0] = Mix_LoadWAV(path);
    if(sounds[SOUND_WALK_0] == NULL){
        printf( "Failed to load 'sound_walk_0'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_walk_1.wav", path);
    sounds[SOUND_WALK_1] = Mix_LoadWAV(path);
    if(sounds[SOUND_WALK_1] == NULL){
        printf( "Failed to load 'sound_walk_1'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_jump.wav", path);
    sounds[SOUND_JUMP] = Mix_LoadWAV(path);
    if(sounds[SOUND_JUMP] == NULL){
        printf( "Failed to load 'sound_jump'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_death.wav", path);
    sounds[SOUND_DEATH] = Mix_LoadWAV(path);
    if(sounds[SOUND_DEATH] == NULL){
        printf( "Failed to load 'sound_death'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_win.wav", path);
    sounds[SOUND_WIN] = Mix_LoadWAV(path);
    if(sounds[SOUND_WIN] == NULL){
        printf( "Failed to load 'sound_win'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
    
    get_game_file_path("Sound/sound_collision.wav", path);
    sounds[SOUND_COLLISION] = Mix_LoadWAV(path);
    if(sounds[SOUND_COLLISION] == NULL){
        printf( "Failed to load 'sound_win'! SDL_mixer Error: %s\n", Mix_GetError() );
        return;
    }
}

void cleanup_sound(){
    for(int i=0; i<sounds_size; i++)
        Mix_FreeChunk(sounds[i]);
    Mix_Quit();
}

void play_sound(u8 sound_index){
    if(!sound_on)
        return;
    Mix_PlayChannel(-1, sounds[sound_index], 0);
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
