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

#define LAGGY 0

int game_mode;

bool should_update_level, should_new_level;
bool game_started;

int init_game(){
    char path[MAX_PATH_LENGTH];
    get_game_file_path("Textures/Quicksand.ttf", path);
    load_font(path);
    
    load_world();
    init_openGL();
    init_sound();
    
    process_menu(0);
    game_mode = GAME_MODE_MENU;
    menu_selected_option = 0;
    while(disabled_options[menu_selected_option]) menu_selected_option++;
    
    player.game_time = 0.f;
    
    return 1;
}

void new_game(){
    game_started = true;
    
    // New game
    player.lives = 3;
    player.time = 0.f;
    
    init_messages();
    load_level(0);
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
int game_loop(bool keys[KEYS_NUM], StaticArray<Event, MAX_EVENTS_PER_LOOP> events){
#if MEASURE_TIME
    auto lastFrame = std::chrono::high_resolution_clock::now();
#endif
    
    reset_memory_pool(temporary_storage);
    
    for(int i=0; i<events.size; i++){
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
                    load_level(current_level+1);
                    should_new_level = true;
                    right_pressed = true;
                }
            }else{
                right_pressed = false;
            }
            if(keys[KEYS_LEFT]){
                if(!left_pressed){
                    load_level(current_level-1);
                    should_new_level = true;
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
        
        player.keys[0] = keys[KEYS_LEFT];
        player.keys[1] = keys[KEYS_RIGHT];
        player.keys[2] = keys[KEYS_SPACE] | keys[KEYS_UP];
#if DEVMODE
    }
#if LAGGY
    player.game_time += TIME_STEP;
    static int next_full_slot = 0;
    static int next_free_slot = 0;
    static u8 last_keys = 0, practical_keys = 0;
    const float lag_delay = 2.f/3.f;
    struct FrameKeyInfo {
        float time;
        u8 keys;
    };
    const int slots = 100;
    static FrameKeyInfo frame_keys[slots] = {{-1.f, 0}};
    
    if(last_keys != current_keys){
        frame_keys[next_free_slot] = {player.game_time+lag_delay, current_keys};
        next_free_slot = (next_free_slot + 1) % slots;
        frame_keys[next_free_slot] = {-1.f, 0};
        last_keys = current_keys;
    }
    
    while(0 < frame_keys[next_full_slot].time && frame_keys[next_full_slot].time < player.game_time){
        practical_keys = frame_keys[next_full_slot].keys;
        next_full_slot = (next_full_slot + 1) % slots;
    }
#else
    u8 practical_keys = current_keys;
#endif
    if(game_mode == GAME_MODE_PLAY){
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
        if(paused && !go_on)
            return 1;
#endif
        player.time += TIME_STEP;
        player.level_time += TIME_STEP;
        player.game_time += TIME_STEP;
#endif
        process_movement(practical_keys);
        process_messages(player.game_time);
        
#if MEASURE_TIME
        auto thisFrame = std::chrono::high_resolution_clock::now();
        logic_time += ((std::chrono::duration<double, std::milli>)(thisFrame-lastFrame)).count()/10.;
#endif
        
        // if(...) should_update_level = true;
    }else{
        process_menu(practical_keys);
    }
    
#if OS != OS_IOS && OS != OS_WASM
    catalog_update();
#endif
    
    {
        static double added_time = 0.f;
        static auto last_frame = std::chrono::high_resolution_clock::now();
        auto this_frame = std::chrono::high_resolution_clock::now();
        added_time = ((std::chrono::duration<double, std::milli>)(this_frame-last_frame)).count()*0.001-TIME_STEP;
        last_frame = this_frame;
        if(added_time > TIME_STEP){
            added_time -= TIME_STEP;
            game_loop(keys, events);
        }
    }
    
    return 1;
}
void game_draw(){
#if MEASURE_TIME
    static auto lastFrame = std::chrono::high_resolution_clock::now();
    static std::chrono::duration<double, std::milli> time_elapsed;
    static unsigned int framesThisSecond = 0;
    static float timeThisSecond = 0.f;
    
    framesThisSecond++;
    auto thisFrame = std::chrono::high_resolution_clock::now();
    time_elapsed = thisFrame-lastFrame;
    lastFrame = thisFrame;
    timeThisSecond += time_elapsed.count();
    
    if(timeThisSecond > 1000.f){
        printf("FPS: %i (logic: %lg%%, drawing: %lg%%)\n", framesThisSecond, logic_time, drawing_time);
        framesThisSecond = 0;
        timeThisSecond = 0.f;
        logic_time = 0.f;
        drawing_time = 0.f;
#if MEASURE_RENDER_TIME
        print_render_time();
#endif
    }
#endif
    
    if(game_mode == GAME_MODE_PLAY){
        draw_scene(should_update_level);
        should_update_level = false;
        should_new_level = false;
    }else{
        draw_menu();
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

#if OS != OS_WASM
#define STB_IMAGE_IMPLEMENTATION
#include "basecode/Include/stb_image.h"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "basecode/Include/stb_truetype.h"
