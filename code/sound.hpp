#ifndef sound_hpp
#define sound_hpp

#include "basecode/sdl2.hpp"
#if OS == OS_WASM
#include <SDL/SDL_mixer.h>
#elif OS == OS_WINDOWS
#include <SDL2/SDL_mixer.h>
#else
#include <SDL2_mixer/SDL_mixer.h>
#endif

const u8 SOUND_WALK_0    = 0;
const u8 SOUND_WALK_1    = 1;
const u8 SOUND_JUMP      = 2;
const u8 SOUND_DEATH     = 3;
const u8 SOUND_WIN       = 4;
const u8 SOUND_COLLISION = 5;
const u8 SOUND_SLIDE     = 6;
const u8 SOUND_SPIKES    = 7;
const u8 SOUND_TICK_0    = 8;
const u8 SOUND_TICK_1    = 9;

const int sounds_size = 10;
struct SoundInfo {
    bool on = true;
    Mix_Chunk *sounds[sounds_size];
};

void init_sound(SoundInfo *sound);
void cleanup_sound(SoundInfo *sound);
void play_sound(SoundInfo *sound, u8 sound_index);


#endif /* sound_hpp */
