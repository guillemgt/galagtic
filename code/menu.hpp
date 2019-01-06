#ifndef menu_hpp
#define menu_hpp

#include "render.hpp"

void select_menu_option();
#if !OS_IS_MOBILE
void process_menu(GameState *game_state, u8 keys);
#else
void process_menu(GameState *game_state, Vec2 point);
void select_menu(GameState *game_state, Vec2 point);
#endif
void draw_menu(GameState *game_state);

#if OS_IS_DESKTOP
const int menu_total_options = 6;
#else
const int menu_total_options = 5;
#endif

extern int menu_selected_option;
extern bool disabled_options[menu_total_options];

#endif /* menu_hpp */
