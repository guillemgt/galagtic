#ifndef main_h
#define main_h

#include "world.hpp"
#include "player.hpp"

extern "C" {
    void c_close_menu();
    void c_open_menu();
#if OS == OS_WASM
    void set_option(int option, int value);
#endif
}

void new_game();

extern bool should_update_level, should_new_level;
extern bool game_started;
extern int game_mode;
const int GAME_MODE_PLAY  = 0;
const int GAME_MODE_MENU  = 1;
const int GAME_MODE_START = 2;

const float visual_lag_time = 0.666f;
const float particle_lag_time = 0.66f;
const float sound_lag_time = 0.66f;

#define DEVMODE 1

#endif /* main_h */

/*
 
 TODO LIST
 * Make visiting short distance images work
 * Add quality "slider"
 * Enable fancy lights disabling
 * Enable Reflections disabling
 * Add wall at the ends of the gallery
 */
