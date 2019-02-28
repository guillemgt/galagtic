#ifndef _LAG_BASECODE_HPP
#define _LAG_BASECODE_HPP

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
    float time;
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
    float draw_new_state_time = -1.f, draw_new_level_time = -1.f;
    float input_lag_time, render_lag_time;
    
    // Messages
    int next_free_particle_slot = 0, last_read_particle_slot = 0;
    ParticleMessage particle_messages[MAX_UPS];
    int next_free_sound_slot = 0, last_read_sound_slot = 0;
    SoundMessage       sound_messages[MAX_UPS];
    
    // Levels
    int levels_num;
    LevelInfo level_infos[max_levels];
    
    // Stats
    GameStats stats;
    
    //
    GLObjects gl_objects;
    
    //
    MenuInfo menu_info;
    
    SoundInfo sound;
    
    // Game state
    int game_mode;
    
    bool should_save_game;
    bool game_started, game_started_this_run;
    
    int loaded_save_level;
};

#endif