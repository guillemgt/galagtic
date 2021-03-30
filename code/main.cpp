#include <chrono>

#define GGTP_PROGRAM_STATE GameState
#include "include/ggt_platform.h"

#include "include/stb_image.h"
#include "include/stb_truetype.h"
#if OS == OS_WINDOWS
#include "include/cute_sound.h"
#endif

#include "include/ggt_gl_utils.h"
#include "include/ggt_math.h"
#include "include/misc_tools.hpp"

#include "basecode.hpp"


#define MEASURE_TIME 0
#define MEASURE_RENDER_TIME 0
#define DEBUG_PAUSE 0


#include "main.hpp"
#include "render.hpp"
#include "sound.hpp"
#include "menu.hpp"



#if SOUND_ENGINE == SOUND_ENGINE_CUTE
#define CUTE_SOUND_IMPLEMENTATION
#include "include/cute_sound.h"
#endif


#include "levels.c"
#include "player.cpp"
#include "render.cpp"
#include "sound.cpp"
#include "world.cpp"
#include "menu.cpp"

#define LAGGY 1
const float lag_time = 0.8f;

GameState *global_game_state;

Vec2i window_size; //TODO remove this
float TIME_STEP;

// * On the web version, if there is lag it does weird jumps!

int ggtp_init(GameState *game_state){
	TIME_STEP = 1.f / 60.f;
	window_size.x = 600;
	window_size.y = 600;
    
    allocate_temporary_memory(KB(13990L));
    
    
    float dpi_factor;
    if(ggtp_create_window(600, 600, "lagoon moon base", &dpi_factor) == GGT_FAILURE){
        printf("Error: Couldn't create window\n");
        return GGT_FAILURE;
    }
    
    global_game_state = game_state;
    
    char path[MAX_PATH_LENGTH];
    ggtp_program_file_path("Textures/Quicksand.ttf", path);
    load_font_into_texture(path, GL_TEXTURE2);
    
    load_world(game_state);
    init_openGL(game_state);
    init_sound(&game_state->sound);
    
    // Set up the game state
    game_state->time = 0.f;
    load_game(game_state);
    
#if OS == OS_WASM
    if(game_state->high_dpi_enabled) ggtp_toggle_dpi(1);
#endif
    
    // Set up the menu
    game_state->game_mode = GAME_MODE_MENU;
    MenuInfo *menu = &game_state->menu_info;
    menu->screen = MS_MAIN;
    menu->fade_alpha = -1.f;
    
    game_state->ending_animation_info.started = false;
    
    process_menu(game_state);
    menu->selected_option = next_available_menu_option_after(menu, 0);
    
    return GGT_SUCCESS;
}

void new_game(GameState *game_state, bool newer_game){
    game_state->current_run_data.game_started = true;
    game_state->is_in_real_game = true;
    
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
    
    game_state->ending_animation_info.started = false;
    
    load_level(game_state, 0);
    game_state->should_save_game = true;
    
    init_messages(game_state);
    
    game_state->draw_new_level_time = -1.f;
}

void continue_game(GameState *game_state){
    game_state->is_in_real_game = true;
    
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
    
    game_state->ending_animation_info.started = false;
    
    load_level(game_state, crt->level_num);
    game_state->should_save_game = false;
    
    init_messages(game_state);
    
    game_state->draw_new_level_time = -1.f;
}

void new_level_select_game(GameState *game_state, int num, bool newer_game){
    game_state->is_in_real_game = false;
    
    // New game
    game_state->player_lives = STARTING_LIVES;
    game_state->time = 0.f;
    game_state->time_started_counting = 0;
    
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
    
    game_state->ending_animation_info.started = false;
    
    load_level(game_state, num);
    
    init_messages(game_state);
    
    game_state->draw_new_level_time = -1.f;
}

void complete_game(GameState *game_state){ // Called in player.cpp through a sound message
    // Record highscores
    GameStats *stats = &game_state->stats;
    
    float time = game_state->time - game_state->time_started_counting;
    if(game_state->space_lagged){
        stats->best_time_plus =  MIN(stats->best_time_plus,  time);
        stats->best_lives_plus = MAX(stats->best_lives_plus, game_state->player_lives);
    }else{
        stats->best_time =  MIN(stats->best_time,  time);
        stats->best_lives = MAX(stats->best_lives, game_state->player_lives);
    }
    
    // Put info in EndingAnimationInfo
    EndingAnimationInfo *eai = &game_state->ending_animation_info;
    eai->menu_animation_time = 0.f;
    eai->time = time;
    eai->deaths = STARTING_LIVES - game_state->player_lives;
    eai->started = true;
    
    // Load end animation
    //game_state->game_mode = GAME_MODE_MENU;
    game_state->menu_info.screen = MS_END;
    game_state->menu_info.selected_option = 0;
    game_state->menu_info.fade_alpha = 0.f;
    game_state->menu_info.fade_direction = 0.15f;
    game_state->menu_info.fade_color.r = 255;
    game_state->menu_info.fade_color.g = 255;
    game_state->menu_info.fade_color.b = 255;
    
    // Reset game
    game_state->current_run_data.game_started = false;
    save_game(game_state);
}

extern f32 TIME_STEP;

#if MEASURE_TIME
static double logic_time = 0., drawing_time = 0.;
static int actionsThisSecond = 0;
#endif

void open_menu(GameState *game_state){
    game_state->game_mode = GAME_MODE_MENU;
    game_state->menu_info.screen = MS_INGAME;
    game_state->menu_info.selected_option = next_available_menu_option_after(&game_state->menu_info, 0);
    stop_sound(&game_state->sound, SOUND_MACHINE);
}
void close_menu(GameState *game_state){
    game_state->game_mode = GAME_MODE_PLAY;
    if(game_state->level.num == LAST_LEVEL){
        stop_sound(&game_state->sound, SOUND_MACHINE); // Otherwise it can play twice if we are loading the level at the same time!
        play_sound(&game_state->sound, SOUND_MACHINE);
    }
}

int ggtp_loop(GameState *game_state, ggt_u8 keys[GGTP_TOTAL_KEYS], ggtp_mouse mouse, ggt_platform_events events){
    global_game_state = game_state;
#if MEASURE_TIME
    auto lastFrame = std::chrono::high_resolution_clock::now();
#endif
    
    reset_temporary_memory();
    
    bool was_in_play_mode = game_state->game_mode == GAME_MODE_PLAY;
    
    static bool space_down = false;
    
    for(uint i=0; i<events.size; i++){
        switch(events.data[i].type){
            case GGTP_EVENT_RESIZE: {
				ggt_vec2i v = events.data[i].info.size;
				window_size.x = v.x;
				window_size.y = v.y;
                change_window_size(game_state, Vec2((float)v.x, (float)v.y));
            } break;
            case GGTP_EVENT_CLOSE: {
                return GGT_FAILURE;
            } break;
            case GGTP_EVENT_KEY_DOWN: {
#if DEBUG_BUILD
                if(events.data[i].info.key == 'C'){
                    game_state->stats.best_time      = 1234.56f;
                    game_state->stats.best_time_plus = 99991234.56f;
                    game_state->stats.best_lives      = -1234;
                    game_state->stats.best_lives_plus = -1234;
                    game_state->stats.unlocked_levels      = ArraySize(all_levels);
                    game_state->stats.unlocked_levels_plus = ArraySize(all_levels);
                }
#endif
                if(game_state->game_mode == GAME_MODE_PLAY){
					if(events.data[i].info.key == GGTP_KEY_ESC && !game_state->ending_animation_info.started){
                        if(game_state->is_in_real_game)
                            save_game_into_crt(game_state);
                        open_menu(game_state);
                    }
                }else{
                    if(events.data[i].info.key != GGTP_KEY_SPACE || !space_down)
                        if(!menu_keydown(game_state, events.data[i].info.key))
                        return GGT_FAILURE;
                }
            } break;
            case GGTP_EVENT_MOUSE_MOVE: {
                if(game_state->game_mode == GAME_MODE_MENU){
					ggt_vec2i v = events.data[i].info.mouse_movement.position;
					if(menu_mousemove(game_state, Vec2((float)v.x, (float)v.y)))
						ggtp_set_cursor(GGTP_CURSOR_POINTER);
					else
						ggtp_set_cursor(GGTP_CURSOR_ARROW);
                }
            } break;
            case GGTP_EVENT_MOUSE_LEFT_DOWN: {
                if(game_state->game_mode == GAME_MODE_MENU){
					ggt_vec2i v = events.data[i].info.coords;
                    if(menu_mousemove(game_state, Vec2((float)v.x, (float)v.y)))
                        select_menu_option(game_state);
                }
            } break;
            default: break;
        }
    }
    
    if(keys[GGTP_KEY_SPACE]){
        space_down = true;
    }else{
        space_down = false;
    }
    
    
    u8 current_keys = 0;
#if DEBUG_BUILD
    if(keys[GGTP_KEY_SHIFT] && game_state->game_mode == GAME_MODE_PLAY){
        {
            static bool right_pressed = false, left_pressed = false;
            if(keys[GGTP_KEY_RIGHT]){
                if(!right_pressed){
                    load_level(game_state, game_state->level.num+1);
                    game_state->draw_new_level_time = -1.f;
                    right_pressed = true;
                }
            }else{
                right_pressed = false;
            }
            if(keys[GGTP_KEY_LEFT]){
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
        if(keys[GGTP_KEY_UP])    current_keys |= frame_key_up;
        if(keys[GGTP_KEY_RIGHT]) current_keys |= frame_key_right;
        if(keys[GGTP_KEY_DOWN])  current_keys |= frame_key_down;
        if(keys[GGTP_KEY_LEFT])  current_keys |= frame_key_left;
        if(keys[GGTP_KEY_SPACE]) current_keys |= frame_key_space;
#if DEBUG_BUILD
    }
#endif
    if(game_state->game_mode == GAME_MODE_PLAY){
        static clock_t last_actions_time = clock();
        clock_t this_actions_time = clock();
        static double time_since_last_action = 0.f;
        time_since_last_action += (double)(this_actions_time-last_actions_time) / CLOCKS_PER_SEC;
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
            const int slots = 120;
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
                if(game_state->is_in_real_game)
                    save_game(game_state);
                game_state->should_save_game = false;
            }
            
#if DEBUG_PAUSE
            static bool paused = false, pressed_p = false, pressed_n = false;
            
            if(keys['P']){
                if(!pressed_p){
                    pressed_p = true;
                    paused = !paused;
                }
            }else
                pressed_p = false;
            bool go_on = false;
            if(keys['N']){
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
            
            if(game_state->menu_info.screen == MS_END){
                game_state->gl_objects.screen_translate.x = (0.2f + game_state->menu_info.fade_alpha) * (0.5f - (float)rand() / RAND_MAX) + (-0.5f*game_state->gl_objects.shown_level_size.x);
                game_state->gl_objects.screen_translate.y = (0.2f + game_state->menu_info.fade_alpha) * (0.5f - (float)rand() / RAND_MAX) + (-0.5f*game_state->gl_objects.shown_level_size.y);
                if(game_state->menu_info.fade_alpha >= 1.f){
                    game_state->game_mode = GAME_MODE_MENU;
                    game_state->menu_info.fade_alpha = -1.f;
                }
            }else if(game_state->menu_info.screen == MS_LEVEL_SELECT && game_state->menu_info.fade_alpha >= 1.f){
                game_state->game_mode = GAME_MODE_MENU;
                game_state->menu_info.screen = MS_LEVEL_SELECT;
                game_state->menu_info.selected_option = 0;
                game_state->particles.size = 0;
                game_state->menu_info.fade_alpha = 1.f;
                game_state->menu_info.fade_direction = -1.f;
                menu_load_level(game_state);
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
    
    
#if SOUND_ENGINE == SOUND_ENGINE_CUTE
    process_sound(game_state);
#endif
    
    game_state->was_in_play_mode = was_in_play_mode; 
    
    
    return GGT_SUCCESS;
}
void ggtp_draw(GameState *game_state){
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
    
    static clock_t last_frame = clock();
    clock_t this_frame = clock();
    float time_step = (float)(this_frame - last_frame)/CLOCKS_PER_SEC;
    last_frame = this_frame;
    
    if(game_state->game_mode == GAME_MODE_PLAY){
        game_state->gl_objects.viewport_offset.x = 0;
        game_state->gl_objects.viewport_offset.y = 0;
        draw_scene(game_state, time_step, true);
    }else{
        draw_menu(game_state, time_step);
    }
    
#if MEASURE_TIME
    thisFrame = std::chrono::high_resolution_clock::now();
    drawing_time += ((std::chrono::duration<double, std::milli>)(thisFrame-lastFrame)).count()/10.;
#endif
}
void cleanup_game(){
    
}

void draw_scene(GameState *game_state, float time_step, bool should_draw_ui_and_player){
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
    
    draw_scene(game_state, time_step, should_redraw_level, should_draw_ui_and_player);
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

void save_game_into_crt(GameState *game_state){
    CurrentRunData *crt = &game_state->current_run_data;
    crt->level_num = game_state->level.num;
    crt->player_lives = game_state->player_lives;
    crt->time = game_state->time;
    crt->time_started_counting = game_state->time_started_counting;
    crt->space_lagged = game_state->space_lagged;
    crt->game_started = true;
}

#if OS == OS_WASM

extern "C" {
    extern void setCookie(const char *name, const char *value);
    extern char *getCookie(const char *name);
}

#define fwrite(ptr, size, q, fp) \
for(int i=0; i<size*q; i++){ \
    fp[fpp++] = 'A' + ((((u8 *)ptr)[i] >> 0) & 0x0f); \
    fp[fpp++] = 'A' + ((((u8 *)ptr)[i] >> 4) & 0x0f); \
}
#define fread(ptr, size, q, fp) \
for(int i=0; i<size*q; i++){ \
    ((u8 *)ptr)[i] = fp[fpp++] - 'A'; \
    ((u8 *)ptr)[i] |= (fp[fpp++] - 'A') << 4; \
}

#endif

void save_game(GameState *game_state){
    if(game_state->game_mode == GAME_MODE_PLAY && game_state->is_in_real_game && game_state->current_run_data.game_started) save_game_into_crt(game_state); // If this is in the real game (and before the ending, put all the current game data into the crt struct
    
#if OS != OS_WASM
    char path[MAX_PATH_LENGTH];
    ggtp_user_file_path("save", path);
    
    FILE *fp = fopen(path, "wb");
    if(fp == nullptr){
        printf("Error: Couldn't write save file\n");
        return;
    }
#else
    char fp[1024];
    int fpp = 0;
#endif
    
#define SAVECHECK 1234
    int savecheck = SAVECHECK;
    
    fwrite(&savecheck, sizeof(int), 1, fp);
    
    fwrite(&game_state->sound.on, sizeof(bool), 1, fp);
#if OS == OS_WASM
    fwrite(&game_state->high_dpi_enabled, sizeof(bool), 1, fp);
#endif
    
    fwrite(&game_state->stats, sizeof(GameStats), 1, fp);
    i8 level;
    CurrentRunData *crt = &game_state->current_run_data;
    if(game_state->current_run_data.game_started){
        level = (i8)crt->level_num;
        fwrite(&level, sizeof(i8), 1, fp);
        fwrite(&crt->time, sizeof(crt->time), 1, fp);
        fwrite(&crt->time_started_counting, sizeof(crt->time_started_counting), 1, fp);
        fwrite(&crt->player_lives, sizeof(crt->player_lives), 1, fp);
        fwrite(&crt->space_lagged, sizeof(crt->space_lagged), 1, fp);
    }else{
        level = -1;
        fwrite(&level, sizeof(i8), 1, fp);
    }
    
    fwrite(&savecheck, sizeof(int), 1, fp);
    
#if OS != OS_WASM
    fclose(fp);
#else
    fp[fpp] = 0;
    setCookie("lmb-save", fp);
#endif
}

void reset_save(GameState *game_state){
    game_state->stats.best_time            = INFINITY;
    game_state->stats.best_time_plus       = INFINITY;
    game_state->stats.best_lives           = INT_MIN;
    game_state->stats.best_lives_plus      = INT_MIN;
    game_state->stats.unlocked_levels      = 0;
    game_state->stats.unlocked_levels_plus = 0;
    game_state->current_run_data.game_started = false;
    game_state->sound.on = true;
#if OS == OS_WASM
    game_state->high_dpi_enabled = true;
#endif
}

void load_game(GameState *game_state){
#if OS != OS_WASM
    char path[MAX_PATH_LENGTH];
    ggtp_user_file_path("save", path);
    FILE *fp = fopen(path, "rb");
    if(fp == nullptr){
#else
        int fpp = 0;
        char *fp = getCookie("lmb-save");
        if(fp[0] == 0 || strcmp(fp, "(null)") == 0){
#endif
            reset_save(game_state);
            return;
        }
        
        int savecheck;
        fread(&savecheck, sizeof(int), 1, fp);
        if(savecheck != SAVECHECK){
            reset_save(game_state);
            return;
        }
        
        
        
        fread(&game_state->sound.on, sizeof(bool), 1, fp);
#if OS == OS_WASM
        fread(&game_state->high_dpi_enabled, sizeof(bool), 1, fp);
#endif
        
        fread(&game_state->stats, sizeof(GameStats), 1, fp);
        i8 level;
        fread(&level, sizeof(i8), 1, fp);
        
        CurrentRunData *crt = &game_state->current_run_data;
        if(level >= 0){
            crt->level_num = level;
            crt->game_started = true;
            fread(&crt->time, sizeof(crt->time), 1, fp);
            fread(&crt->time_started_counting, sizeof(crt->time_started_counting), 1, fp);
            fread(&crt->player_lives, sizeof(crt->player_lives), 1, fp);
            fread(&crt->space_lagged, sizeof(crt->space_lagged), 1, fp);
        }else
            crt->game_started = false;
        
        
        fread(&savecheck, sizeof(int), 1, fp);
        if(savecheck != SAVECHECK || (crt->game_started && (crt->level_num < 0 || crt->level_num > LAST_LEVEL))){
            reset_save(game_state);
            return;
        }
        
#if OS != OS_WASM
        fclose(fp);
#else
        free(fp);
#endif
    }
    
    
#if OS == OS_WASM
#undef fwrite
#undef fread
#endif
    
    
    //#ifndef __EMSCRIPTEN__
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
    //#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/stb_truetype.h"
    
#define GGT_PLATFORM_IMPLEMENTATION
#include "include/ggt_platform.h"
#define GGT_GL_IMPLEMENTATION
#include "include/ggt_gl_utils.h"
#define MISC_TOOLS_IMPLEMENTATION
#include "include/misc_tools.hpp"