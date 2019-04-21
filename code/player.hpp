#ifndef player_hpp
#define player_hpp

#include <time.h>
#include "render.hpp"

struct Player {
    Vec2 r, v;
    u32 flags;
    int at_platform;
    float jump_height, time_to_unstick, gravity_extra;
    
    bool space_was_up = false;
};
struct PlayerSnapshot {
    Vec2 r, v;
    float time;
    u32 flags;
};

struct GameStats {
    float best_time, best_time_plus;
    int best_lives, best_lives_plus, unlocked_levels;
};
struct CurrentRunData {
    int level_num, player_lives;
    float time, time_started_counting;
    bool space_lagged, game_started;
};


const u32 PM_PUSHES_OBJECT_RIGHT  = 0x00000001;
const u32 PM_PUSHES_OBJECT_LEFT   = 0x00000002;
const u32 PM_PUSHES_OBJECT_BOTTOM = 0x00000004;
const u32 PM_PUSHES_OBJECT_TOP    = 0x00000008;
const u32 PM_PUSHED_OBJECT_RIGHT  = 0x00000010;
const u32 PM_PUSHED_OBJECT_LEFT   = 0x00000020;
const u32 PM_PUSHED_OBJECT_BOTTOM = 0x00000040;
const u32 PM_PUSHED_OBJECT_TOP    = 0x00000080;
const u32 PM_PUSHES_TILE_RIGHT  = 0x00000100;
const u32 PM_PUSHES_TILE_LEFT   = 0x00000200;
const u32 PM_PUSHES_TILE_BOTTOM = 0x00000400;
const u32 PM_PUSHES_TILE_TOP    = 0x00000800;
const u32 PM_PUSHED_TILE_RIGHT  = 0x00001000;
const u32 PM_PUSHED_TILE_LEFT   = 0x00002000;
const u32 PM_PUSHED_TILE_BOTTOM = 0x00004000;
const u32 PM_PUSHED_TILE_TOP    = 0x00008000;
const u32 PM_PUSHES_RIGHT  = PM_PUSHES_OBJECT_RIGHT  | PM_PUSHES_TILE_RIGHT;
const u32 PM_PUSHES_LEFT   = PM_PUSHES_OBJECT_LEFT   | PM_PUSHES_TILE_LEFT;
const u32 PM_PUSHES_BOTTOM = PM_PUSHES_OBJECT_BOTTOM | PM_PUSHES_TILE_BOTTOM;
const u32 PM_PUSHES_TOP    = PM_PUSHES_OBJECT_TOP    | PM_PUSHES_TILE_TOP;
const u32 PM_PUSHED_RIGHT  = PM_PUSHED_OBJECT_RIGHT  | PM_PUSHED_TILE_RIGHT;
const u32 PM_PUSHED_LEFT   = PM_PUSHED_OBJECT_LEFT   | PM_PUSHED_TILE_LEFT;
const u32 PM_PUSHED_BOTTOM = PM_PUSHED_OBJECT_BOTTOM | PM_PUSHED_TILE_BOTTOM;
const u32 PM_PUSHED_TOP    = PM_PUSHED_OBJECT_TOP    | PM_PUSHED_TILE_TOP;
const u32 PM_ON_GROUND = PM_PUSHES_BOTTOM;

const u32 PM_STICKING_TO_WALL_RIGHT = 0x00010000;
const u32 PM_STICKING_TO_WALL_LEFT  = 0x00020000;
const u32 PM_WALL_JUMPING_RIGHT     = 0x00040000;
const u32 PM_WALL_JUMPING_LEFT      = 0x00080000;
const u32 PM_STICKING_TO_WALL = PM_STICKING_TO_WALL_RIGHT | PM_STICKING_TO_WALL_LEFT;
const u32 PM_WALL_JUMPING = PM_WALL_JUMPING_RIGHT | PM_WALL_JUMPING_LEFT;
const u32 PM_DISABLED               = 0x80000000;
const u32 PM_LOOKING_LEFT           = 0x00100000;

struct SoundMessage {
    float time;
    u8 sound;
};
struct ParticleMessage {
    Vec2 r, v;
    float time;
    bool is_death;
};


const u8 frame_key_up    = 0x01;
const u8 frame_key_right = 0x02;
const u8 frame_key_down  = 0x04;
const u8 frame_key_left  = 0x08;
const u8 frame_key_space = 0x10;
const u8 frame_key_jump  = 0x11;
const float load_next_level_in_max = 1.5f;

const Vec2 enemies_speed = Vec2(-2.f, 0.f);

struct Platform {
    Vec2 r, size, v;
};

struct Particle {
    Vec2 r, v;
    RgbaColor color;
};

const int MAX_ENEMIES            = 16;
const int MAX_PLATFORMS          =  8;
const int MAX_GATES              = 32;
const int MAX_RETRACTABLE_SPIKES = 32;

#define MAX_PARTICLES 9000
#define MAX_UPS 120 // Updates Per Second

const u8 GF_HORIZONTAL = 0x01;
const u8 GF_VISITED    = 0x02;
const u8 GF_IMMORTAL   = 0x04;
struct Gate {
    Vec2 r;
    float state;
    u8 flags;
};
struct RetractableSpike {
    Vec2 r;
    float state;
};

struct Level {
    int width, height;
    char layout[max_level_width][max_level_height];
    StaticArray<Vec2, MAX_ENEMIES> enemies;
    StaticArray<Platform, MAX_PLATFORMS> platforms;
    float time_to_load_next_level, time_to_load_next_level_max, time;
    Vec2 start_r, goal_r;
    bool state, completed;
    int num, exit_side;
};


#define SIDE_UP    1
#define SIDE_RIGHT 2
#define SIDE_DOWN  3
#define SIDE_LEFT  4

#define WATER_SUBDIVISIONS 8
#define WATER_MAX_X 12
#define WATER_NODES (WATER_MAX_X*WATER_SUBDIVISIONS)
struct LaggedLevel {
    Vec2 key_r;
    StaticArray<Vec2, MAX_PLATFORMS> platform_sizes;
    StaticArray<Gate, MAX_GATES>     gates;
    StaticArray<RetractableSpike, MAX_RETRACTABLE_SPIKES> retractable_spikes;
    float water_height[WATER_MAX_X*WATER_SUBDIVISIONS], water_speed[WATER_MAX_X*WATER_SUBDIVISIONS];
};



struct PlatformInfo {
    Vec2 r, v;
};

struct LevelInfo {
    StaticArray<Vec2, MAX_ENEMIES>           enemies;
    StaticArray<PlatformInfo, MAX_PLATFORMS> platforms;
    Vec2i size;
    Vec2 start, start_first_time;
    char layout[max_level_width][max_level_height];
    int exit_side;
};
extern LevelInfo all_levels[];

void add_snapshot(GameState *game_state);

void change_level_state(Level *level);
bool change_level_state_and_return_if_changed(Level *level);
void reset_player(LevelInfo *level_info, Player *player, Level *level);

void load_player_into_buffer(GameState *game_state, BufferAndCount *buffer);
void load_particles_into_buffer(GameState *game_state, BufferAndCount *buffer);

void process_movement(GameState *game_state, u8 keys);
void init_messages(GameState *game_state);
void process_messages(GameState *game_state);

void find_snapshot_to_render(GameState *game_state);

struct PlayerRenderingInfo {
    int current_frame_x = 0;
    int current_frame_y = 0;
    int time_in_current_frame = 0;
};

#endif /* player_hpp */
