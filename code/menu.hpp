#ifndef menu_hpp
#define menu_hpp

#include "render.hpp"

void process_menu(u8 keys);
void draw_menu();

#if OS_IS_DESKTOP
const int menu_total_options = 5;
#else
const int menu_total_options = 4;
#endif

extern int menu_selected_option;
extern bool disabled_options[menu_total_options];

#endif /* menu_hpp */
