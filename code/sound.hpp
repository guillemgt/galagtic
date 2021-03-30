#ifndef sound_hpp
#define sound_hpp

#if OS == OS_WASM
#include <SDL/SDL_mixer.h>
#endif

/*#include "basecode/sdl2.hpp"
#if OS == OS_WASM
#include <SDL/SDL_mixer.h>
#elif OS == OS_WINDOWS
#include <SDL2/SDL_mixer.h>
#else
#include <SDL2_mixer/SDL_mixer.h>
#endif*/

#define SOUND_ENGINE_JS   1
#define SOUND_ENGINE_SDL  2
#define SOUND_ENGINE_CUTE 3

#if OS == OS_WASM
#define SOUND_ENGINE SOUND_ENGINE_JS
#elif defined(MIX_MAX_VOLUME)
#define SOUND_ENGINE SOUND_ENGINE_SDL
#elif defined(CUTE_SOUND_H)
#define SOUND_ENGINE SOUND_ENGINE_CUTE
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
const u8 SOUND_MACHINE   = 10;
const u8 SOUND_HIGHPITCH = 11;
const int sounds_size    = 12;
struct SoundInfo {
    bool on = true;
#if SOUND_ENGINE == SOUND_ENGINE_SDL
    Mix_Chunk *sounds[sounds_size];
#elif SOUND_ENGINE == SOUND_ENGINE_CUTE
    cs_context_t *ctx;
    cs_loaded_sound_t audios[sounds_size];
    cs_play_sound_def_t sounds[sounds_size];
#endif
};

void init_sound(SoundInfo *sound);
void cleanup_sound(SoundInfo *sound);
void play_sound(SoundInfo *sound, u8 sound_index);
void stop_sound(SoundInfo *sound, u8 sound_index);

#if SOUND_ENGINE == SOUND_ENGINE_CUTE
void process_sound(GameState *game_state);
#endif

#endif /* sound_hpp */
