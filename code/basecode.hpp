#ifndef _LAG_BASECODE_HPP
#define _LAG_BASECODE_HPP

#define BASECODE_WINDOW_NAME "game of lag"

struct Level;

#include "render.hpp"
#include "world.hpp"
#include "player.hpp"
#include "menu.hpp"
#include "sound.hpp"
#include "main.hpp"

struct GameState {
    Player player;
    int player_lives;
    float time, time_started_counting;
    Level level;
    
    // Things for the deferred rendering
    LaggedLevel next_lagged_level, lagged_level;
    PlayerSnapshot rendered_player;
    float player_interpolation_factor;
    int next_free_snapshot = 0;
    int last_rendered_snapshot = 0;
    PlayerSnapshot                      player_snapshots[MAX_UPS];
    StaticArray<Vec2, MAX_ENEMIES>     enemies_snapshots[MAX_UPS];
    StaticArray<Vec2, MAX_PLATFORMS> platforms_snapshots[MAX_UPS];
    u8 completion_snapshots[MAX_UPS];
    StaticArray<Particle, MAX_PARTICLES> particles;
    float draw_new_state_time[2] = {INFINITY, INFINITY}, draw_new_level_time = -1.f;
    i8 draw_new_state_state[2] = {-1, -1};
    float input_lag_time, render_lag_time;
    
    // Messages
    int next_free_particle_slot = 0, last_read_particle_slot = 0;
    ParticleMessage particle_messages[MAX_UPS];
    int next_free_sound_slot = 0, last_read_sound_slot = 0;
    SoundMessage       sound_messages[MAX_UPS];
    
    // Stats
    GameStats stats;
    CurrentRunData current_run_data;
    
    //
    GLObjects gl_objects;
    PlayerRenderingInfo player_rendering_info;
    EnemyRenderingInfo enemy_rendering_info;
    
    //
    MenuInfo menu_info;
    
    SoundInfo sound;
    
    EndingAnimationInfo ending_animation_info;
    
    // Game state
    int game_mode;
    
    bool was_in_play_mode = false;
    bool should_save_game, space_lagged;
    bool is_in_real_game; // ie not in level select mode
    
#if OS == OS_WASM
    bool high_dpi_enabled;
#endif
};

#endif