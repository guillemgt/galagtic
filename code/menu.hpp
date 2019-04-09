#ifndef menu_hpp
#define menu_hpp

#include "render.hpp"

typedef enum {
    MS_MAIN          = 0,
    MS_INGAME        = 1,
    MS_LEVEL_SELECT  = 2,
    MS_SETTINGS      = 3,
    MS_SURE_NEW_GAME = 4,
} MenuScreen;

enum {
    MB_MAIN_CONTINUE,
    MB_MAIN_NEW_GAME,
    MB_MAIN_NEW_GAME_PLUS,
    MB_MAIN_LEVEL_SELECT,
    MB_MAIN_SETTINGS,
    MB_MAIN_QUIT,
};
enum {
    MB_INGAME_RESUME,
    MB_INGAME_SETTINGS,
    MB_INGAME_TO_MAIN,
    MB_INGAME_QUIT,
};
enum {
    MB_LS_PLAY,
    MB_LS_PLAY_PLUS,
    MB_LS_BACK,
    MB_LS_RIGHT,
    MB_LS_LEFT
};
enum {
    MB_SETTINGS_SOUND,
    MB_SETTINGS_BACK,
};

const int menu_total_screens = 5;

int select_menu_option(GameState *game_state);
int menu_keydown(GameState *game_state, char key);
int menu_mousemove(GameState *game_state, Vec2 position);
void process_menu(GameState *game_state);
void draw_menu(GameState *game_state);

const int menu_max_options = 6;

#if OS_IS_DESKTOP
const int menu_total_options[5] = {6, 4, 3, 2, 1};
#else
const int menu_total_options[5] = {5, 3, 3, 2, 1};
#endif

struct MenuInfo {
    MenuScreen screen, last_screen;
    
    int selected_option, last_selected_option, shown_level;
    bool disabled_options[menu_max_options];
    float options_width[menu_max_options];
    
    float fade_alpha, fade_direction;
};


#endif /* menu_hpp */
