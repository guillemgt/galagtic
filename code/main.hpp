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

void new_game(bool newer_game);
void continue_game();
void save_game();
void load_game();

extern bool should_update_level, should_new_level, should_save_game;
extern bool game_started, game_started_this_run;
extern int game_mode;
const int GAME_MODE_PLAY  = 0;
const int GAME_MODE_MENU  = 1;
const int GAME_MODE_START = 2;

extern float input_lag_time, visual_lag_time, particle_lag_time, sound_lag_time;

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
