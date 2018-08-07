#ifndef player_hpp
#define player_hpp

#include <time.h>
#include "render.hpp"

struct Player {
    Vec2 r, v, safe_r;
    float load_next_level_in, time, level_time, game_time, jump_height, cancel_next_level_in;
    int lives;
    u8 sticking_to_wall;
    bool on_ground, completed_level;
    bool keys[3];
};
struct GameStats {
    clock_t starting_time;
    unsigned int deaths;
};
extern Player player, rendered_player;
extern GameStats game_stats;
extern float level_moving_y;
extern int level_state;

struct SoundMessage {
    float time;
    u8 sound;
};
struct ParticleMessage {
    Vec2 r;
    float time;
    bool is_death;
};


void load_player_into_buffer(BufferAndCount *buffer);
void load_particles_into_buffer(BufferAndCount *buffer);

const u8 frame_key_up    = 0x01;
const u8 frame_key_right = 0x02;
const u8 frame_key_down  = 0x04;
const u8 frame_key_left  = 0x08;
const u8 frame_key_space = 0x10;
const u8 frame_key_jump  = 0x11;
const float load_next_level_in_max = 1.5f;

void process_movement(u8 keys);
void init_messages();
void process_messages(float time);

const int MAX_ENEMIES = 64;
extern StaticArray<Vec2, MAX_ENEMIES> level_enemies[max_levels];
extern StaticArray<Vec2, MAX_ENEMIES> enemies;


#endif /* player_hpp */
