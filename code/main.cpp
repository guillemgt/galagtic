#include <chrono>


#include "basecode/basecode.hpp"
#if OS != OS_IOS
#define USE_SDL 1
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

int game_mode;

bool should_save_game;
bool game_started, game_started_this_run;

float input_lag_time, visual_lag_time, particle_lag_time, sound_lag_time;

int loaded_save_level;

int init_game(GameState *game_state){
    char path[MAX_PATH_LENGTH];
    get_game_file_path("Textures/Quicksand.ttf", path);
    load_font(path);
    
    load_world(game_state);
    init_openGL();
    init_sound();
    
    process_menu(game_state, 0);
    game_mode = GAME_MODE_MENU;
    menu_selected_option = 0;
    while(disabled_options[menu_selected_option]) menu_selected_option++;
    
    game_state->game_time = 0.f;
    game_state->time = 0.f;
    game_state->stats.best_time = -1.f;
    game_state->stats.best_lives = 9;
    game_started_this_run = false;
    load_game(game_state);
    
    return 1;
}

void new_game(GameState *game_state, bool newer_game){
    game_started = true;
    game_started_this_run = true;
    
    // New game
    game_state->player_lives = 3;
    game_state->game_time = 0.f;
    //player.space_lagged = newer_game;
#if 0
    input_lag_time = 0.f;
    visual_lag_time = 0.f;
    particle_lag_time = 0.f;
    sound_lag_time = 0.f;
#else
    if(newer_game){
        input_lag_time = 2.f/3.f;
        visual_lag_time = 0.f;
        particle_lag_time = 0.f;
        sound_lag_time = 0.f;
    }else{
        input_lag_time = 0.f;
        visual_lag_time = 2.f/3.f;
        particle_lag_time = 2.f/3.f;
        sound_lag_time = 2.f/3.f;
    }
#endif
    
    load_level(game_state, 0);
    should_save_game = true;
    
    init_messages(game_state);
}

void continue_game(GameState *game_state){
    game_started = true;
    game_started_this_run = true;
    
    game_state->game_time = 0.f;
    
    load_level(game_state, loaded_save_level);
    should_save_game = false;
    
    init_messages(game_state);
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
        game_mode = GAME_MODE_PLAY;
#if OS == OS_WASM
        js_hide_menu();
#endif
    }
    
    void c_open_menu(){
        game_mode = GAME_MODE_MENU;
        menu_selected_option = 0;
        while(disabled_options[menu_selected_option]) menu_selected_option++;
#if OS == OS_WASM
        js_show_menu();
#endif
    }
}

extern BufferAndCount picture_buffer;
int game_loop(GameState *game_state, bool keys[KEYS_NUM], StaticArray<Event, MAX_EVENTS_PER_LOOP> events){
#if MEASURE_TIME
    auto lastFrame = std::chrono::high_resolution_clock::now();
#endif
    
    reset_memory_pool(temporary_storage);
    
    for(uint i=0; i<events.size; i++){
        switch(events[i].id){
            case EVENT_RESIZE: {
                change_window_size();
            } break;
        }
    }
    
    static bool esc_pressed = false;
    if(keys[KEYS_ESC]){
        if(!esc_pressed){
            if(game_mode == GAME_MODE_PLAY)
                c_open_menu();
            else
                c_close_menu();
            esc_pressed = true;
        }
    }else{
        esc_pressed = false;
    }
    
    u8 current_keys = 0;
#if DEVMODE
    if(keys[KEYS_SHIFT] && game_mode == GAME_MODE_PLAY){
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
    }
    if(game_mode == GAME_MODE_PLAY){
        static auto last_actions_time = std::chrono::high_resolution_clock::now();
        auto this_actions_time = std::chrono::high_resolution_clock::now();
        static double time_since_last_action = 0.f;
        time_since_last_action += ((std::chrono::duration<double, std::milli>)(this_actions_time-last_actions_time)).count()*0.001;
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
                frame_keys[next_free_slot] = {game_state->game_time+input_lag_time, current_keys};
                next_free_slot = (next_free_slot + 1) % slots;
                frame_keys[next_free_slot] = {-1.f, 0};
                last_keys = current_keys;
            }
            while(0 <= frame_keys[next_full_slot].time && frame_keys[next_full_slot].time <= game_state->game_time){
                practical_keys = frame_keys[next_full_slot].keys;
                next_full_slot = (next_full_slot + 1) % slots;
            }
            if(false) //player.space_lagged)
                practical_keys = (practical_keys & (0xff ^ frame_key_space)) | (current_keys & frame_key_space);
#else
            u8 practical_keys = current_keys;
#endif
            
            if(should_save_game){
                save_game(game_state);
                should_save_game = false;
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
            game_state->game_time += TIME_STEP;
            
            process_movement(game_state, practical_keys);
            
            time_since_last_action -= TIME_STEP;
#if MEASURE_TIME
            actionsThisSecond++;
#endif
        }
        game_state->player_interpolation_factor = (f32)(time_since_last_action / TIME_STEP); // We render 1 frame behind... I guess it doesn't matter...
        
#if MEASURE_TIME
        auto thisFrame = std::chrono::high_resolution_clock::now();
        logic_time += ((std::chrono::duration<double, std::milli>)(thisFrame-lastFrame)).count()/10.;
#endif
        
        // if(...) should_update_level = true;
    }else{
#if !OS_IS_MOBILE
        process_menu(game_state, current_keys);
#else
        extern Vec2 touch_point;
        process_menu(touch_point);
#endif
    }
    
#if OS != OS_IOS && OS != OS_WASM
    catalog_update();
#endif
    
    
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
        //printf("FPS: %i/%i (logic: %lg%%, drawing: %lg%%)\n", actionsThisSecond, framesThisSecond, logic_time, drawing_time);
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
    
    if(game_mode == GAME_MODE_PLAY){
        bool should_redraw_level = false;
        bool should_redraw_state = false;
        if(game_state->draw_new_state_time < game_state->game_time){
            game_state->draw_new_state_time = INFINITY;
            should_redraw_state = true;
        }
        if(game_state->draw_new_level_time < game_state->game_time){
            game_state->draw_new_level_time = INFINITY;
            should_redraw_level = true;
        }
        draw_scene(game_state, should_redraw_level, should_redraw_state);
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

extern "C" {
    void set_option(int option, int value){
        switch(option){
            case 0: // Sound
            sound_on = value;
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
    bool unlocked_newer_game = false;
    write_file(&unlocked_newer_game, sizeof(bool), 1, fp);
    i8 level;
    if(game_started){
        level = (i8)game_state->level.num;
        write_file(&level, sizeof(i8), 1, fp);
        write_file(&game_state->game_time, sizeof(game_state->game_time), 1, fp);
        write_file(&game_state->player_lives, sizeof(game_state->player_lives), 1, fp);
        //write_file(&player.space_lagged, sizeof(player.space_lagged), 1, fp);
    }else{
        level = -1;
        write_file(&level, sizeof(i8), 1, fp);
    }
}
void load_game(GameState *game_state){
    OsFile fp = open_user_file("save", "rb");
    if(fp == nullptr)
        return;
    bool unlocked_newer_game = false;
    read_file(&unlocked_newer_game, sizeof(bool), 1, fp);
    i8 level;
    read_file(&level, sizeof(i8), 1, fp);
    if(level >= 0){
        loaded_save_level = level;
        game_started = true;
        read_file(&game_state->game_time, sizeof(game_state->game_time), 1, fp);
        read_file(&game_state->player_lives, sizeof(game_state->player_lives), 1, fp);
        //read_file(&player.space_lagged, sizeof(player.space_lagged), 1, fp);
    }else
        game_started = false;
}

#if OS != OS_WASM
#define STB_IMAGE_IMPLEMENTATION
#include "basecode/Include/stb_image.h"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "basecode/Include/stb_truetype.h"
