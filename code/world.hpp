#ifndef world_hpp
#define world_hpp

#include "render.hpp"

const u16 BLOCK_IS_SOLID              = 0x0001;
const u16 BLOCK_IS_HARMFUL_RIGHT      = 0x0002;
const u16 BLOCK_IS_HARMFUL_UP         = 0x0004;
const u16 BLOCK_IS_HARMFUL_LEFT       = 0x0008;
const u16 BLOCK_IS_HARMFUL_DOWN       = 0x0010;
const u16 BLOCK_IS_HARMFUL_CENTER     = 0x0020;
const u16 BLOCK_IS_GOAL               = 0x0040;
const u16 BLOCK_IS_TRANSPARENT        = 0x0080;
const u16 BLOCK_IS_RANDOM             = 0x0100;
const u16 BLOCK_HAS_TWO_LAYERS        = 0x0200;
const u16 BLOCK_STOPS_PLATFORMS       = 0x0400;
const u16 BLOCK_HAS_TWO_LAYERS_SQUARE = 0x0800;

const u16 BLOCK_IS_HARMFUL = BLOCK_IS_HARMFUL_CENTER | BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_HARMFUL_UP | BLOCK_IS_HARMFUL_LEFT | BLOCK_IS_HARMFUL_DOWN;

const RgbaColor basic_color = {35, 35, 35, 255};
const RgbaColor water_color = {230, 255, 255, 140};

const int max_level_width  = 30;
const int max_level_height = 25;
const int max_levels = 25;

const float time_after_flag_is_down = 0.5f;

//const RgbaColor level_color = {150, 180, 200, 255};
const RgbaColor level_color = {200, 200, 200, 255};
extern u16  block_info[256];

void load_world(GameState *game_state);
void load_level_into_buffer(GameState *game_state, BufferAndCount *buffer);
void load_changing_level_into_buffer(Level *level, BufferAndCount *buffer);
void load_goal_into_buffer(GameState *game_state, BufferAndCount *buffer);
void load_planet_background(Level *level, BufferAndCount *buffer);

void load_level(GameState *game_state, int num);

const int triangles_per_enemy = 7;
struct EnemyRenderingInfo {
    Vec3 triangle_positions[triangles_per_enemy][3];
    float triangle_periods[triangles_per_enemy][3];
    float flag_t = 0.f;
};

#endif /* world_hpp */
