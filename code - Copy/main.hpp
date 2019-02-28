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

void new_game(GameState *game_state, bool newer_game);
void continue_game(GameState *game_state);
void save_game(GameState *game_state);
void load_game(GameState *game_state);

const int GAME_MODE_PLAY  = 0;
const int GAME_MODE_MENU  = 1;
const int GAME_MODE_START = 2;

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
