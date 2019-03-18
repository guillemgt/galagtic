#include <chrono>

#include "basecode/basecode.hpp"
#if OS != OS_IOS
//#define USE_SDL 1
#endif
#include "basecode/window.hpp"
#include "basecode/opengl.hpp"
#include "basecode/os.hpp"

#include "main.hpp"
#include "render.hpp"
#include "sound.hpp"
#include "menu.hpp"

#include "basecode/basecode.cpp"

#include "player.cpp"
#include "render.cpp"
#include "sound.cpp"
#include "world.cpp"
#include "menu.cpp"

#define LAGGY 1
const float lag_time = 0.8f;

GameState *global_game_state;

int init_game(GameState *game_state){
    global_game_state = game_state;
    
    char path[MAX_PATH_LENGTH];
    get_game_file_path("Textures/Quicksand.ttf", path);
    load_font(path);
    
    load_world(game_state);
    init_openGL(game_state);
    init_sound(&game_state->sound);
    
    // Set up the game state
    game_state->time = 0.f;
    load_game(game_state);
    
    // Set up the menu
    game_state->game_mode = GAME_MODE_MENU;
    MenuInfo *menu = &game_state->menu_info;
    menu->screen = MS_MAIN;
    menu->fade_alpha = -1.f;
    
    process_menu(game_state);
    menu->selected_option = 0;
    while(menu->disabled_options[menu->selected_option]) menu->selected_option++;
    printf("%i\n", menu->selected_option);
    
    return 1;
}

void new_game(GameState *game_state, bool newer_game){
    game_state->current_run_data.game_started = true;
    
    // New game
    game_state->player_lives = 3;
    game_state->time = 0.f;
    game_state->time_started_counting = INFINITY;
    
    game_state->space_lagged = newer_game;
#if !LAGGY
    game_state->input_lag_time = 0.f;
    game_state->render_lag_time = 0.f;
#else
    if(game_state->space_lagged){
        game_state->input_lag_time = lag_time;
        game_state->render_lag_time = 0.f;
    }else{
        game_state->input_lag_time = 0.f;
        game_state->render_lag_time = lag_time;
    }
#endif
    
    load_level(game_state, 0);
    game_state->should_save_game = true;
    
    init_messages(game_state);
    
    game_state->draw_new_level_time = -1.f;
}

void continue_game(GameState *game_state){
    CurrentRunData *crt = &game_state->current_run_data;
    game_state->player_lives = crt->player_lives;
    game_state->time = crt->time;
    game_state->time_started_counting = crt->time_started_counting;
    game_state->space_lagged = crt->space_lagged;
    
#if !LAGGY
    game_state->input_lag_time = 0.f;
    game_state->render_lag_time = 0.f;
#else
    if(game_state->space_lagged){
        game_state->input_lag_time = lag_time;
        game_state->render_lag_time = 0.f;
    }else{
        game_state->input_lag_time = 0.f;
        game_state->render_lag_time = lag_time;
    }
#endif
    
    load_level(game_state, crt->level_num);
    game_state->should_save_game = false;
    
    init_messages(game_state);
    
    game_state->draw_new_level_time = -1.f;
}

void complete_game(GameState *game_state){
    GameStats *stats = &game_state->stats;
    
    float time = game_state->time - game_state->time_started_counting;
    if(game_state->space_lagged){
        stats->best_time_plus =  MIN(stats->best_time_plus,  time);
        stats->best_lives_plus = MAX(stats->best_lives_plus, game_state->player_lives);
    }else{
        stats->best_time =  MIN(stats->best_time,  time);
        stats->best_lives = MAX(stats->best_lives, game_state->player_lives);
    }
    game_state->current_run_data.game_started = false;
    
    game_state->draw_new_level_time = -1.f;
}

extern f32 TIME_STEP;

#if OS == OS_WASM
extern "C" {
    extern void js_show_menu();
    extern void js_hide_menu();
}
#endif

#define MEASURE_TIME 1
#define DEBUG_PAUSE 1

#if MEASURE_TIME
static double logic_time = 0., drawing_time = 0.;
static int actionsThisSecond = 0;
#endif

extern "C" {
    void c_close_menu(){
        global_game_state->game_mode = GAME_MODE_PLAY;
#if OS == OS_WASM
        js_hide_menu();
#endif
    }
    
    void c_open_menu(){
        global_game_state->game_mode = GAME_MODE_MENU;
        global_game_state->menu_info.screen = MS_INGAME;
        global_game_state->menu_info.selected_option = 0;
        while(global_game_state->menu_info.disabled_options[global_game_state->menu_info.selected_option]) global_game_state->menu_info.selected_option++;
#if OS == OS_WASM
        js_show_menu();
#endif
    }
}

extern BufferAndCount picture_buffer;
int game_loop(GameState *game_state, bool keys[KEYS_NUM], StaticArray<Event, MAX_EVENTS_PER_LOOP> events){
    global_game_state = game_state;
#if MEASURE_TIME
    auto lastFrame = std::chrono::high_resolution_clock::now();
#endif
    
    reset_memory_pool(temporary_storage);
    
    bool was_in_play_mode = game_state->game_mode == GAME_MODE_PLAY;
    
    for(uint i=0; i<events.size; i++){
        switch(events[i].id){
            case EVENT_RESIZE: {
                change_window_size(game_state, (Vec2)window_size);
            } break;
            case EVENT_CLOSE: {
                return 0;
            } break;
            case EVENT_KEYDOWN: {
                if(game_state->game_mode == GAME_MODE_PLAY){
                    if(events[i].data.key == KEYS_ESC){
                        CurrentRunData *crt = &game_state->current_run_data;
                        crt->level_num = game_state->level.num;
                        crt->player_lives = game_state->player_lives;
                        crt->time = game_state->time;
                        crt->time_started_counting = game_state->time_started_counting;
                        crt->space_lagged = game_state->space_lagged;
                        crt->game_started = true;
                        
                        c_open_menu();
                    }
                }else{
                    if(!menu_keydown(game_state, events[i].data.key))
                        return 0;
                }
            } break;
            case EVENT_LEFTMOUSEMOVE: {
                if(game_state->game_mode == GAME_MODE_MENU){
                    menu_mousemove(game_state, events[i].data.move.r);
                }
            } break;
            case EVENT_LEFTMOUSEDOWN: {
                if(game_state->game_mode == GAME_MODE_MENU){
                    if(menu_mousemove(game_state, (Vec2)events[i].data.coords))
                        select_menu_option(game_state);
                }
            } break;
        }
    }
    
    u8 current_keys = 0;
#if DEVMODE
    if(keys[KEYS_SHIFT] && game_state->game_mode == GAME_MODE_PLAY){
        {
            static bool right_pressed = false, left_pressed = false;
            if(keys[KEYS_RIGHT]){
                if(!right_pressed){
                    load_level(game_state, game_state->level.num+1);
                    game_state->draw_new_level_time = -1.f;
                    right_pressed = true;
                }
            }else{
                right_pressed = false;
            }
            if(keys[KEYS_LEFT]){
                if(!left_pressed){
                    load_level(game_state, game_state->level.num-1);
                    game_state->draw_new_level_time = -1.f;
                    left_pressed = true;
                }
            }else{
                left_pressed = false;
            }
        }
    }else{
#endif
        if(keys[KEYS_UP])    current_keys |= frame_key_up;
        if(keys[KEYS_RIGHT]) current_keys |= frame_key_right;
        if(keys[KEYS_DOWN])  current_keys |= frame_key_down;
        if(keys[KEYS_LEFT])  current_keys |= frame_key_left;
        if(keys[KEYS_SPACE]) current_keys |= frame_key_space;
#if DEVMODE
    }
#endif
    if(game_state->game_mode == GAME_MODE_PLAY){
        static auto last_actions_time = std::chrono::high_resolution_clock::now();
        auto this_actions_time = std::chrono::high_resolution_clock::now();
        static double time_since_last_action = 0.f;
        time_since_last_action += ((std::chrono::duration<double, std::milli>)(this_actions_time-last_actions_time)).count()*0.001;
        if(!game_state->was_in_play_mode){
            time_since_last_action = TIME_STEP;
        }
        last_actions_time = this_actions_time;
        while(time_since_last_action >= TIME_STEP){
#if LAGGY
            static int next_full_slot = 0;
            static int next_free_slot = 0;
            static u8 last_keys = 0, practical_keys = 0;
            struct FrameKeyInfo {
                float time;
                u8 keys;
            };
            const int slots = 100;
            static FrameKeyInfo frame_keys[slots] = {{-1.f, 0}};
            
            if(last_keys != current_keys){
                frame_keys[next_free_slot] = {game_state->time+game_state->input_lag_time, current_keys};
                next_free_slot = (next_free_slot + 1) % slots;
                frame_keys[next_free_slot] = {-1.f, 0};
                last_keys = current_keys;
            }
            while(0 <= frame_keys[next_full_slot].time && frame_keys[next_full_slot].time <= game_state->time){
                practical_keys = frame_keys[next_full_slot].keys;
                next_full_slot = (next_full_slot + 1) % slots;
            }
            if(game_state->space_lagged)
                practical_keys = (practical_keys & (~frame_key_space)) | (current_keys & frame_key_space);
#else
            u8 practical_keys = current_keys;
#endif
            
            if(game_state->should_save_game){
                save_game(game_state);
                game_state->should_save_game = false;
            }
#if DEBUG_PAUSE
            static bool paused = false, pressed_p = false, pressed_n = false;
            
            if(keys[KEYS_P]){
                if(!pressed_p){
                    pressed_p = true;
                    paused = !paused;
                }
            }else
                pressed_p = false;
            bool go_on = false;
            if(keys[KEYS_N]){
                if(!pressed_n){
                    pressed_n = true;
                    go_on = true;
                }
            }else
                pressed_n = false;
            if(paused && !go_on){
                time_since_last_action -= TIME_STEP;
                return 1;
            }
#endif
            game_state->time += TIME_STEP;
            
            process_movement(game_state, practical_keys);
            
            time_since_last_action -= TIME_STEP;
#if MEASURE_TIME
            actionsThisSecond++;
#endif
            
            if(game_state->menu_info.fade_alpha >= 0.f){
                game_state->menu_info.fade_alpha -= fade_speed*TIME_STEP;
            }
        }
        game_state->player_interpolation_factor = (f32)(time_since_last_action / TIME_STEP); // We render 1 frame behind... I guess it doesn't matter...
        
#if MEASURE_TIME
        auto thisFrame = std::chrono::high_resolution_clock::now();
        logic_time += ((std::chrono::duration<double, std::milli>)(thisFrame-lastFrame)).count()/10.;
#endif
        
        // if(...) should_update_level = true;
    }else{
        process_menu(game_state);
#if OS_IS_MOBILE
        extern Vec2 touch_point;
        process_menu(touch_point);
#endif
    }
    
#if OS != OS_IOS && OS != OS_WASM
    catalog_update();
#endif
    
    game_state->was_in_play_mode = was_in_play_mode; 
    
    
    return 1;
}
void game_draw(GameState *game_state){
#if MEASURE_TIME
    static auto lastFrame = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double, std::milli> time_elapsed;
    static unsigned int framesThisSecond = 0;
    static float timeThisSecond = 0.f;
    
    framesThisSecond++;
    auto thisFrame = std::chrono::high_resolution_clock::now();
    time_elapsed = thisFrame-lastFrame;
    lastFrame = thisFrame;
    timeThisSecond += (f32)time_elapsed.count();
    
    if(timeThisSecond > 1000.f){
        printf("FPS: %i/%i (logic: %lg%%, drawing: %lg%%)\n", actionsThisSecond, framesThisSecond, logic_time, drawing_time);
        framesThisSecond = 0;
        actionsThisSecond = 0;
        timeThisSecond = 0.f;
        logic_time = 0.f;
        drawing_time = 0.f;
#if MEASURE_RENDER_TIME
        print_render_time();
#endif
    }
#endif
    
    if(game_state->game_mode == GAME_MODE_PLAY){
        draw_scene(game_state, true);
    }else{
        draw_menu(game_state);
    }
    
#if MEASURE_TIME
    thisFrame = std::chrono::high_resolution_clock::now();
    drawing_time += ((std::chrono::duration<double, std::milli>)(thisFrame-lastFrame)).count()/10.;
#endif
}
void cleanup_game(){
    
}

void draw_scene(GameState *game_state, bool should_draw_ui_and_player){
    bool should_redraw_level = false;
    
    if(game_state->draw_new_level_time < game_state->time){
        game_state->draw_new_level_time = INFINITY;
        game_state->draw_new_state_time[0] = INFINITY;
        game_state->draw_new_state_time[1] = INFINITY;
        should_redraw_level = true;
    }
    for(int i=0; i<2; i++){
        if(game_state->draw_new_state_time[i] < game_state->time){
            game_state->draw_new_state_time[i] = INFINITY;
            game_state->gl_objects.drawn_level_state = game_state->draw_new_state_state[i];
        }
    }
    draw_scene(game_state, should_redraw_level, should_draw_ui_and_player);
}

extern "C" {
    void set_option(int option, int value){
        switch(option){
            case 0: // Sound
            //sound->on = value;
            break;
        }
    }
}

void save_game(GameState *game_state){
    OsFile fp = open_user_file("save", "wb");
    if(fp == nullptr){
        printf("Error: Couldn't write save file\n");
        return;
    }
    write_file(&game_state->stats, sizeof(GameStats), 1, fp);
    i8 level;
    if(game_state->current_run_data.game_started){
        level = (i8)game_state->level.num;
        write_file(&level, sizeof(i8), 1, fp);
        write_file(&game_state->time, sizeof(game_state->time), 1, fp);
        write_file(&game_state->time_started_counting, sizeof(game_state->time_started_counting), 1, fp);
        write_file(&game_state->player_lives, sizeof(game_state->player_lives), 1, fp);
        write_file(&game_state->space_lagged, sizeof(game_state->space_lagged), 1, fp);
    }else{
        level = -1;
        write_file(&level, sizeof(i8), 1, fp);
    }
    close_file(fp);
}
void load_game(GameState *game_state){
    OsFile fp = open_user_file("save", "rb");
    if(fp == nullptr){
        game_state->stats.best_time       = INFINITY;
        game_state->stats.best_time_plus  = INFINITY;
        game_state->stats.best_lives      = INT_MIN;
        game_state->stats.best_lives_plus = INT_MIN;
        game_state->stats.unlocked_levels = 0;
        game_state->current_run_data.game_started = false;
        return;
    }
    read_file(&game_state->stats, sizeof(GameStats), 1, fp);
    i8 level;
    read_file(&level, sizeof(i8), 1, fp);
    
    CurrentRunData *crt = &game_state->current_run_data;
    if(level >= 0){
        crt->level_num = level;
        crt->game_started = true;
        read_file(&crt->time, sizeof(crt->time), 1, fp);
        read_file(&crt->time_started_counting, sizeof(crt->time_started_counting), 1, fp);
        read_file(&crt->player_lives, sizeof(crt->player_lives), 1, fp);
        read_file(&crt->space_lagged, sizeof(crt->space_lagged), 1, fp);
    }else
        crt->game_started = false;
    
    close_file(fp);
}

#if OS != OS_WASM
#define STB_IMAGE_IMPLEMENTATION
#include "basecode/Include/stb_image.h"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "basecode/Include/stb_truetype.h"
