#ifndef world_hpp
#define world_hpp

#include "render.hpp"

const u8 BLOCK_IS_SOLID       = 0x01;
const u8 BLOCK_IS_HARMFUL     = 0x02;
const u8 BLOCK_IS_GOAL        = 0x04;
const u8 BLOCK_IS_TRANSPARENT = 0x08;
const u8 BLOCK_IS_RANDOM      = 0x10;
const u8 BLOCK_HAS_TWO_LAYERS = 0x20;

const RgbaColor basic_color = {35, 35, 35, 255};

const int max_level_width  = 30;
const int max_level_height = 20;
const int max_levels = 25;

extern int  level_width;
extern int  level_height;
extern char **level_layout, **level_moving_layout;
const RgbaColor level_color = {200, 200, 200, 255};
extern u8   block_info[256];
extern Vec2 goal_position;

extern int levels_num;
extern int current_level;

void load_world();
void load_level_into_buffer(BufferAndCount *buffer, BufferAndCount *t_buffer);
void load_changing_level_into_buffer(BufferAndCount *buffer);
void load_goal_into_buffer(BufferAndCount *buffer, BufferAndCount *loading_buffer);
void load_level(int num);

#endif /* world_hpp */
