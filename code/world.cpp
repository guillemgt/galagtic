#include "include/ggt_math.h"
#include "include/misc_tools.hpp"

#include "world.hpp"
#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

u16 block_info[256];

const float flag_speed = 3.f;

extern Vec2i window_size;

void load_world(GameState *game_state){
    block_info[' ']  = BLOCK_IS_TRANSPARENT;
    block_info['#']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS;
    
    block_info[')']  = BLOCK_IS_TRANSPARENT;
    block_info['@']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS;
    block_info['F']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS | BLOCK_HAS_TWO_LAYERS;
    
    block_info['H']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['h']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['j']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['G']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['g']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['J']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['k']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    block_info['K']  = BLOCK_IS_SOLID | BLOCK_STOPS_PLATFORMS | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS_SQUARE;
    
    block_info['^']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_TRANSPARENT;
    block_info['v']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_TRANSPARENT;
    block_info['>']  = BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info['<']  = BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['/']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info[']'] = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['7']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['1']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info['U']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['D']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['R']  = BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['L']  = BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['X']  = BLOCK_IS_TRANSPARENT;
    block_info['Z']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['x']  = BLOCK_IS_HARMFUL_UP | BLOCK_IS_TRANSPARENT;
    block_info['z']  = BLOCK_IS_HARMFUL_UP | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['*']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT;
    block_info['+']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['%']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT;
    block_info['a']  = BLOCK_IS_TRANSPARENT;
    block_info['b']  = BLOCK_IS_TRANSPARENT | BLOCK_IS_SOLID | BLOCK_IS_HARMFUL_CENTER | BLOCK_IS_HARMFUL_RIGHT  | BLOCK_IS_HARMFUL_LEFT;
    block_info['.']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['-']  = BLOCK_IS_TRANSPARENT | BLOCK_STOPS_PLATFORMS;
    
    // Things for rendering
    EnemyRenderingInfo *eri = &game_state->enemy_rendering_info;
    for(int i=0; i<triangles_per_enemy; i++){
        float angle = 2.f*M_PI*(float)rand()/RAND_MAX;
        for(int j=0; j<3; j++){
            float r = 0.15f+0.35f*(float)rand()/RAND_MAX;
            float c = cosf(angle);
            float s = sinf(angle);
            eri->triangle_positions[i][j] = Vec3(r*c, r*s, -0.1f*(float)rand()/RAND_MAX);
            eri->triangle_periods[i][j] = 0.35f*sqrtf(r);
            angle += 2.f*M_PI/3.f;
        }
    }
}

void load_level(GameState *game_state, int num){
    if(num < 0 || num >= ArraySize(all_levels)) num = 0;
    
    if(num > 0 && game_state->time_started_counting == INFINITY)
        game_state->time_started_counting = game_state->time + game_state->render_lag_time;
    
    if(game_state->is_in_real_game){ // MAke sure we don't update these things from the level select screen in the menu!!
        if(game_state->stats.unlocked_levels < num)
            game_state->stats.unlocked_levels = num;
        
        if(game_state->space_lagged && game_state->stats.unlocked_levels_plus < num)
            game_state->stats.unlocked_levels_plus = num;
    }
    
    //printf("Loading level #%i/%lli\n", num, ArrayCount(all_levels)-1);
    
    Player *player = &game_state->player;
    Level  *level  = &game_state->level;
    
    LevelInfo *level_info = &all_levels[num];//= game_state->level_infos + num;
    LaggedLevel *lagged_level = &game_state->next_lagged_level;
    
    level->num = num;
    lagged_level->num = num;
    for(int x=0; x<level_info->size.x; x++){
        for(int y=0, y2=level_info->size.y-1; y2>=0; y++, y2--){
            level->layout[x][y2] = level_info->layout[y][x];
        }
    }
    
    level->width  = level_info->size.x;
    level->height = level_info->size.y;
    level->exit_side = level_info->exit_side;
    
    lagged_level->width  = (float)level->width;
    lagged_level->height = (float)level->height;
    
    level->state = 0;
    
    level_size = Vec2((f32)level->width, (f32)level->height);
    
    lagged_level->gates.size = 0;
    lagged_level->retractable_spikes.size = 0;
    
    for(int x=0; x<level->width; x++){
        for(int y=0; y<level->height; y++){
            char c = level->layout[x][y];
            if(c == '*' || c == '%' || c == '+'){
                level->goal_r = Vec2(x+0.5f, y+0.5f);
            }else if(c == 'H' || c == 'F'){
                lagged_level->gates.push({Vec2((f32)x, (f32)y), 0.5f, GF_HORIZONTAL});
            }else if(c == 'j'){
                lagged_level->gates.push({Vec2((f32)x, (f32)y), 0.f, GF_HORIZONTAL | GF_IMMORTAL});
            }else if(c == 'G'){
                lagged_level->gates.push({Vec2((f32)x, (f32)y), 0.5f, 0});
            }else if(c == 'J'){
                lagged_level->gates.push({Vec2((f32)x, (f32)y), 0.f, GF_IMMORTAL});
            }else if(c == 'X' || c == 'Z'){
                lagged_level->retractable_spikes.push({Vec2((f32)x, (f32)y), -0.5f});
            }
        }
    }
    level->start_r = level_info->start;
    
    level->enemies.size = level_info->enemies.size;
    for(uint i=0; i<level->enemies.size; i++)
        level->enemies[i] = level_info->enemies[i];
    
    lagged_level->platform_sizes.size = level_info->platforms.size;
    level->platforms.size = level_info->platforms.size;
    for(uint i=0; i<level->platforms.size; i++){
        level->platforms[i] = level_info->platforms[i];
        lagged_level->platform_sizes[i] = level_info->platforms[i].size;
    }
    
    int y, x = (int)level->goal_r.x;
    for(y=(int)level->goal_r.y; y>=0; y--){
        if(level->layout[x][y] == '#')
            break;
    }
    lagged_level->key_r = level->goal_r;
    
    reset_player(level_info, player, level);
    
    player->r = level_info->start_first_time;
    
    if(num == 0){
        for(int i=0; i<WATER_NODES_1; i++){
            lagged_level->water_height[i] = 1.8f;
            lagged_level->water_speed[i]  = 0.f;
        }
    }
}


float some_rand(float x, float y){
    return 1.f+0.5f*(sinf(5.f*x)+cosf(8.f*y));
}

void load_level_into_buffer(GameState *game_state, BufferAndCount *buffer, BufferAndCount *changing_buffer_0, BufferAndCount *changing_buffer_1){
    Level *level = &game_state->level;
    int level_num = level->num;
    auto &layout = level->layout;
    uint vertex_count = 4*6;
    uint changing_vertex_count_0 = 0;
    uint changing_vertex_count_1 = 0;
    
    {
        for(int x=0; x<level->width; x++){
            for(int y=0; y<level->height; y++){
                char c = layout[x][y];
                if(block_info[c] & BLOCK_IS_TRANSPARENT)
                    vertex_count += 6;
                if(c == '#' || c == '@' || c == ')')
                    vertex_count += 6;
                else if(c == '/' || c == ']' || c == '7' || c == '1')
                    vertex_count += 18;
                else if(c == 'F')
                    vertex_count += 9;
                else if(c == 'a'){
                    changing_vertex_count_0 += 12;
                    changing_vertex_count_1 += 6;
                }else if(c == 'b'){
                    changing_vertex_count_0 += 6;
                    changing_vertex_count_1 += 12;
                }else if(block_info[c] & BLOCK_IS_HARMFUL)
                    vertex_count += 9;
                //else if((c == '*' || c == '%') && !player.completed_level)
                //    vertex_count += 6;
            }
        }
        
        
        start_temp_alloc();
        Vertex_PCa *o_vertices = (Vertex_PCa *)talloc(vertex_count*sizeof(Vertex_PCa));
        Vertex_PCa *o_changing_vertices_0 = (Vertex_PCa *)talloc(changing_vertex_count_0*sizeof(Vertex_PCa));
        Vertex_PCa *o_changing_vertices_1 = (Vertex_PCa *)talloc(changing_vertex_count_1*sizeof(Vertex_PCa));
        Vertex_PCa *vertices = o_vertices;
        Vertex_PCa *changing_vertices_0 = o_changing_vertices_0;
        Vertex_PCa *changing_vertices_1 = o_changing_vertices_1;
        
        {
            const RgbaColor color = basic_color;
        }
        
        float fx = 0.f;
        for(int x=0; x<level->width; x++){
            float fy = 0.f;
            for(int y=0; y<level->height; y++){
                char c = layout[x][y];
                switch(c){
                    case '#': {
                        const RgbaColor color = basic_color;
                        const float disp = 0.05f;
                        float r0 = disp*((17*x+13*y)%23)/23.f;
                        float r1 = disp*((13*x+17*y)%23)/23.f;
                        float r2 = disp*((19*x+11*y)%23)/23.f;
                        float r3 = disp*((11*x+19*y)%23)/23.f;
                        Vec3 square[4] = {
                            Vec3(fx+0.f-r0, fy+0.f-r1, 0.f),
                            Vec3(fx+1.f+r1, fy+0.f-r3, 0.f),
                            Vec3(fx+0.f-r2, fy+1.f+r0, 0.f),
                            Vec3(fx+1.f+r3, fy+1.f+r2, 0.f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                        
                    } break;
                    case '@': {
                        i8 r = rand()%8;
                        RgbaColor color = {(u8)(255-r), (u8)(244-r), (u8)(224-r), 255};
                        const float disp = 0.05f;
                        float r0 = disp*((17*x+13*y)%23)/23.f;
                        float r1 = disp*((13*x+17*y)%23)/23.f;
                        float r2 = disp*((19*x+11*y)%23)/23.f;
                        float r3 = disp*((11*x+19*y)%23)/23.f;
                        Vec3 square[4] = {
                            Vec3(fx+0.f-r0, fy+0.f-r1, 0.f),
                            Vec3(fx+1.f+r1, fy+0.f-r3, 0.f),
                            Vec3(fx+0.f-r2, fy+1.f+r0, 0.f),
                            Vec3(fx+1.f+r3, fy+1.f+r2, 0.f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    } break;
                    case ')': {
                        RgbaColor color = water_color;
                        Vec3 square[4] = {
                            Vec3(fx+0.f, fy+0.f,  0.f),
                            Vec3(fx+1.f, fy+0.f,  0.f),
                            Vec3(fx+0.f, fy+0.8f, 0.f),
                            Vec3(fx+1.f, fy+0.8f, 0.f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    } break;
                    case '^':
                    case 'U':
                    case 'x':{
                        const RgbaColor color = basic_color;
                        float r0 = 0.3f+0.2f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.3f+0.2f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+0.f, 0.f), color};
                    } break;
                    case 'v':
                    case 'D':
                    case 'y': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.7f-0.2f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.7f-0.2f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+1.f, 0.f), color};
                    } break;
                    case '>':
                    case 'R':
                    case 'z': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.3f+0.2f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.3f+0.2f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case '<':
                    case 'L':
                    case 'w': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.7f-0.2f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.7f-0.2f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+1.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case '/': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.25f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.25f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+0.f, 0.f), color};
                    } {
                        const RgbaColor color = basic_color;
                        float r0 = 0.25f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.25f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case ']': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.25f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.25f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+0.f, 0.f), color};
                    } {
                        const RgbaColor color = basic_color;
                        float r0 = 0.5f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.5f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+1.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case '1': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.5f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.5f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+1.f, 0.f), color};
                    } {
                        const RgbaColor color = basic_color;
                        float r0 = 0.25f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.25f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case '7': {
                        const RgbaColor color = basic_color;
                        float r0 = 0.5f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.5f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+0.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+2.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+4.f/6.f, fy+1.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                        *(vertices++) = {Vec3(fx+6.f/6.f, fy+1.f, 0.f), color};
                    } {
                        const RgbaColor color = basic_color;
                        float r0 = 0.5f+0.25f*((13*x+17*y)%23)/23.f;
                        float r1 = 0.5f;
                        float r2 = 0.5f+0.25f*((17*x+13*y)%23)/23.f;
                        *(vertices++) = {Vec3(fx+1.f, fy+0.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r0,  fy+1.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+2.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r1,  fy+3.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+4.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+r2,  fy+5.f/6.f, 0.f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+6.f/6.f, 0.f), color};
                    } break;
                    case 'F': {
                        {
                            RgbaColor color = {188, 183, 214, 255};
                            Vec3 square[4] = {
                                Vec3(fx+0.f, fy+0.f, 0.183f),
                                Vec3(fx+1.f, fy+0.f, 0.183f),
                                Vec3(fx+0.f, fy+1.f, 0.183f),
                                Vec3(fx+1.f, fy+1.f, 0.183f),
                            };
                            *(vertices++) = {square[0], color};
                            *(vertices++) = {square[1], color};
                            *(vertices++) = {square[2], color};
                            *(vertices++) = {square[2], color};
                            *(vertices++) = {square[1], color};
                            *(vertices++) = {square[3], color};
                        }
                        {
                            const RgbaColor color = {0, 0, 0, 20};
                            *(vertices++) = {{fx+0.25f, fy+0.75f, 0.182f}, color};
                            *(vertices++) = {{fx+0.75f, fy+0.75f, 0.182f}, color};
                            *(vertices++) = {{fx+0.5f,  fy+0.25f, 0.182f}, color};
                        }
                    } break;
                }
                
                if(c == 'a' || c == 'b'){
                    Vertex_PCa *verts_0, *verts_1;
                    if(c == 'a'){
                        verts_0 = changing_vertices_0;
                        verts_1 = changing_vertices_1;
                    }else{
                        verts_0 = changing_vertices_1;
                        verts_1 = changing_vertices_0;
                    }
                    
                    const float disp = 0.05f;
                    float r0 = disp*((17*x+13*y)%23)/23.f;
                    float r1 = disp*((13*x+17*y)%23)/23.f;
                    float r2 = disp*((19*x+11*y)%23)/23.f;
                    float r3 = disp*((11*x+19*y)%23)/23.f;
                    {
                        const RgbaColor color = {200, 200, 200, 55};
                        Vec3 square[4] = {
                            Vec3(fx+0.1f-r0, fy+0.1f-r1, 0.18f),
                            Vec3(fx+0.9f+r1, fy+0.1f-r3, 0.18f),
                            Vec3(fx+0.1f-r2, fy+0.9f+r0, 0.18f),
                            Vec3(fx+0.9f+r3, fy+0.9f+r2, 0.18f),
                        };
                        *(verts_0++) = {square[0], color};
                        *(verts_0++) = {square[1], color};
                        *(verts_0++) = {square[2], color};
                        *(verts_0++) = {square[2], color};
                        *(verts_0++) = {square[1], color};
                        *(verts_0++) = {square[3], color};
                    }
                    {
                        const RgbaColor color = {50, 50, 50, 255};
                        Vec3 square[4] = {
                            Vec3(fx+0.05f-r0, fy+0.05f-r1, 0.19f),
                            Vec3(fx+0.95f+r1, fy+0.05f-r3, 0.19f),
                            Vec3(fx+0.05f-r2, fy+0.95f+r0, 0.19f),
                            Vec3(fx+0.95f+r3, fy+0.95f+r2, 0.19f),
                        };
                        *(verts_0++) = {square[0], color};
                        *(verts_0++) = {square[1], color};
                        *(verts_0++) = {square[2], color};
                        *(verts_0++) = {square[2], color};
                        *(verts_0++) = {square[1], color};
                        *(verts_0++) = {square[3], color};
                    }
                    {
                        const RgbaColor color = {50, 50, 50, 255};
                        Vec3 square[4] = {
                            Vec3(fx+0.f-r0, fy+0.f-r1, 0.f),
                            Vec3(fx+1.f+r1, fy+0.f-r3, 0.f),
                            Vec3(fx+0.f-r2, fy+1.f+r0, 0.f),
                            Vec3(fx+1.f+r3, fy+1.f+r2, 0.f),
                        };
                        *(verts_1++) = {square[0], color};
                        *(verts_1++) = {square[1], color};
                        *(verts_1++) = {square[2], color};
                        *(verts_1++) = {square[2], color};
                        *(verts_1++) = {square[1], color};
                        *(verts_1++) = {square[3], color};
                    }
                    
                    if(c == 'a'){
                        changing_vertices_0 = verts_0;
                        changing_vertices_1 = verts_1;
                    }else{
                        changing_vertices_0 = verts_1;
                        changing_vertices_1 = verts_0;
                    }
                }
                
                if(level_num != 0 && (block_info[c] & BLOCK_IS_TRANSPARENT)){
                    if(block_info[c] & BLOCK_HAS_TWO_LAYERS){
                        const RgbaColor color = {176, 172, 200, 255};
                        const float disp = 0.05f;
                        float r0 = disp*((17*x+13*y)%23)/23.f;
                        float r1 = disp*((13*x+17*y)%23)/23.f;
                        float r2 = disp*((19*x+11*y)%23)/23.f;
                        float r3 = disp*((11*x+19*y)%23)/23.f;
                        Vec3 square[4] = {
                            Vec3(fx+0.f-r0, fy+0.f-r1, 0.19f),
                            Vec3(fx+1.f+r1, fy+0.f-r3, 0.19f),
                            Vec3(fx+0.f-r2, fy+1.f+r0, 0.19f),
                            Vec3(fx+1.f+r3, fy+1.f+r2, 0.19f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    }else if(block_info[c] & BLOCK_HAS_TWO_LAYERS_SQUARE){
                        const RgbaColor color = {176, 172, 200, 255};
                        Vec3 square[4] = {
                            Vec3(fx+0.f, fy+0.f, 0.19f),
                            Vec3(fx+1.f, fy+0.f, 0.19f),
                            Vec3(fx+0.f, fy+1.f, 0.19f),
                            Vec3(fx+1.f, fy+1.f, 0.19f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    }else{
                        const RgbaColor color = {196, 191, 222, 255};
                        *(vertices++) = {Vec3(fx+0.f, fy+0.f, 0.2f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+0.f, 0.2f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+1.f, 0.2f), color};
                        *(vertices++) = {Vec3(fx+0.f, fy+1.f, 0.2f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+0.f, 0.2f), color};
                        *(vertices++) = {Vec3(fx+1.f, fy+1.f, 0.2f), color};
                    }
                }
                fy += 1.f;
            }
            fx += 1.f;
        }
        
        /*
        vertices = o_vertices;
        const RgbaColor color = {255, 1, 2, 255}; //
        *(vertices++) = {Vec3(0.f, 0.f, 0.2f), color};
        *(vertices++) = {Vec3((float)level->width + 1.f, 0.f, 0.2f), color};
        *(vertices++) = {Vec3(0.f, (float)level->height + 1.f, 0.2f), color};
        *(vertices++) = {Vec3(0.f, (float)level->height + 1.f, 0.2f), color};
        *(vertices++) = {Vec3((float)level->width + 1.f, 0.f, 0.2f), color};
        *(vertices++) = {Vec3((float)level->width + 1.f, (float)level->height + 1.f, 0.2f), color};
        */
        
        
        buffer->count = (u32)(vertices-o_vertices);
        assert(buffer->count <= vertex_count);
        ggtgl_set_buffer_data(buffer->buffer, o_vertices, buffer->count, GL_STATIC_DRAW);
        
        changing_buffer_0->count = (u32)(changing_vertices_0 - o_changing_vertices_0);
        assert(changing_buffer_0->count <= changing_vertex_count_0);
        if(changing_buffer_0->count > 0) 
            ggtgl_set_buffer_data(changing_buffer_0->buffer, o_changing_vertices_0, changing_buffer_0->count, GL_STATIC_DRAW);
        
        
        changing_buffer_1->count = (u32)(changing_vertices_1 - o_changing_vertices_1);
        assert(changing_buffer_1->count <= changing_vertex_count_1);
        if(changing_buffer_1->count > 0) 
            ggtgl_set_buffer_data(changing_buffer_1->buffer, o_changing_vertices_1, changing_buffer_1->count, GL_STATIC_DRAW);
        
        end_temp_alloc();
    }
    
    
    
    
    game_state->gl_objects.shown_level_size = level_size;
    game_state->gl_objects.screen_translate = -0.5f*game_state->gl_objects.shown_level_size; // This is copied in main.cpp
}

void load_planet_background(Level *level, BufferAndCount *buffer){
    if(level->num > 0){
        buffer->count = 0;
        return;
    }
    
    const float z = 0.185f;
    Vertex_PT p[] = {
        {{0.f,               0.f,                z}, {0.f, 0.f}},
        {{0.f,               (f32)level->height, z}, {0.f, 1.f}},
        {{(f32)level->width, 0.f,                z}, {1.f, 0.f}},
        {{(f32)level->width, 0.f,                z}, {1.f, 0.f}},
        {{0.f,               (f32)level->height, z}, {0.f, 1.f}},
        {{(f32)level->width, (f32)level->height, z}, {1.f, 1.f}},
    };
    
    buffer->count = 6;
    glBindBuffer(GL_ARRAY_BUFFER, buffer->buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(p), p, GL_STATIC_DRAW);
}

void load_dynamic_level_parts_into_buffer(GameState *game_state, BufferAndCount *buffer, float time_step){
    const float water_subdivision_distance = 1.f/WATER_SUBDIVISIONS;
    
    Vec3 (*triangle_positions)[3] = &game_state->enemy_rendering_info.triangle_positions[0];
    float (*triangle_periods)[3] = &game_state->enemy_rendering_info.triangle_periods[0];
    {
        start_temp_alloc();
        
        auto &enemies_snapshot = game_state->enemies_snapshots[game_state->last_rendered_snapshot];
        auto &platforms_snapshot = game_state->platforms_snapshots[game_state->last_rendered_snapshot];
        u8 level_completed = game_state->completion_snapshots[game_state->last_rendered_snapshot];
        Vec2 player_r = game_state->player_snapshots[game_state->last_rendered_snapshot].r;
        
        LaggedLevel *level = &game_state->lagged_level;
        
#define CORE_TRIANGLES 25
        int vertex_count = 3*triangles_per_enemy*(enemies_snapshot.size) + 6*platforms_snapshot.size + 6*WATER_NODES_1 + (level->num == ArraySize(all_levels)-1 ? 3*CORE_TRIANGLES : 0);
        
        Vertex_PCa *o_vertices = (Vertex_PCa *)talloc(vertex_count*sizeof(Vertex_PCa));
        Vertex_PCa *vertices = o_vertices;
        
        { // Enemies
            RgbaColor color = {0, 0, 0, 255};
            for(uint k=0; k<enemies_snapshot.size; k++){
                Vec2 p = enemies_snapshot[k];
                for(int i=0; i<triangles_per_enemy; i++){
                    for(int j=0; j<3; j++){
                        float t = game_state->time/triangle_periods[i][j];
                        float s = cosf(t);
                        *(vertices++) = {Vec3(p.x+s*triangle_positions[i][j].x, p.y+s*triangle_positions[i][j].y, 0.01f), color};
                    }
                }
            }
        }
        { // Gates
            RgbaColor color = {35, 35, 35, 255};
            for(uint k=0; k<level->gates.size; k++){
                Gate *g = &level->gates[k];
                Vec2 r = g->r;
                if(g->flags & GF_VISITED && g->flags & GF_IMMORTAL){
                    if(g->state > 0.f && g->state < 0.5f){
                        g->state = fminf(g->state + 2.f*time_step, 0.5f);
                    }
                }else if(level_completed && g->state > 0.f){
                    if(g->state >= 0.5f && level->num > 0)
                        play_sound(&game_state->sound, SOUND_SLIDE);
                    g->state = fmaxf(g->state - 2.f*time_step, 0.f);
                }
                
                
                float s = g->state;
                float t = 1.f-s;
                
                //printf("%g %g\n", s, t);
                //fflush(stdout);
                
                const float z = -0.1f;
                
                if(g->flags & GF_HORIZONTAL){
                    float y0 = r.y;
                    float y1 = r.y+1.f;
                    float xa0 = r.x;
                    float xa1 = r.x+s;
                    float xb0 = r.x+t;
                    float xb1 = r.x+1.f;
                    Vec3 box[8] = {
                        Vec3(xa0, y0, z),
                        Vec3(xa1, y0, z),
                        Vec3(xa0, y1, z),
                        Vec3(xa1, y1, z),
                        
                        Vec3(xb0, y0, z),
                        Vec3(xb1, y0, z),
                        Vec3(xb0, y1, z),
                        Vec3(xb1, y1, z),
                    };
                    *(vertices++) = {box[0], color};
                    *(vertices++) = {box[1], color};
                    *(vertices++) = {box[2], color};
                    *(vertices++) = {box[2], color};
                    *(vertices++) = {box[1], color};
                    *(vertices++) = {box[3], color};
                    
                    *(vertices++) = {box[4], color};
                    *(vertices++) = {box[5], color};
                    *(vertices++) = {box[6], color};
                    *(vertices++) = {box[6], color};
                    *(vertices++) = {box[5], color};
                    *(vertices++) = {box[7], color};
                }else{
                    Vec3 box[8] = {
                        Vec3(r.x+0.f, r.y+0.f, z),
                        Vec3(r.x+1.f, r.y+0.f, z),
                        Vec3(r.x+0.f, r.y+s,   z),
                        Vec3(r.x+1.f, r.y+s,   z),
                        
                        Vec3(r.x+0.f, r.y+t,   z),
                        Vec3(r.x+1.f, r.y+t,   z),
                        Vec3(r.x+0.f, r.y+1.f, z),
                        Vec3(r.x+1.f, r.y+1.f, z),
                    };
                    *(vertices++) = {box[0], color};
                    *(vertices++) = {box[1], color};
                    *(vertices++) = {box[2], color};
                    *(vertices++) = {box[2], color};
                    *(vertices++) = {box[1], color};
                    *(vertices++) = {box[3], color};
                    
                    *(vertices++) = {box[4], color};
                    *(vertices++) = {box[5], color};
                    *(vertices++) = {box[6], color};
                    *(vertices++) = {box[6], color};
                    *(vertices++) = {box[5], color};
                    *(vertices++) = {box[7], color};
                }
            }
        }
        { // Retractable spikes
            const RgbaColor color = basic_color;
            bool do_sound = false;
            
            for(uint k=0; k<level->retractable_spikes.size; k++){
                RetractableSpike *g = &level->retractable_spikes[k];
                float fx = g->r.x, fy = g->r.y;
                int x = (int)fx, y = (int)fy;
                
                if(level_completed){
                    if(g->state <= -0.5f)
                        do_sound = true;
                    if(g->state < 0.f)
                        g->state += 2.f*time_step;
                    else
                        g->state = 0.f;
                }else
                    g->state = -0.5f;
                
                fy += g->state;
                float r0 = 0.3f+0.2f*((13*x+17*y)%23)/23.f;
                float r1 = 0.5f;
                float r2 = 0.3f+0.2f*((17*x+13*y)%23)/23.f;
                *(vertices++) = {Vec3(fx+0.f/6.f, fy+0.f, 0.f), color};
                *(vertices++) = {Vec3(fx+1.f/6.f, fy+r0,  0.f), color};
                *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                *(vertices++) = {Vec3(fx+2.f/6.f, fy+0.f, 0.f), color};
                *(vertices++) = {Vec3(fx+3.f/6.f, fy+r1,  0.f), color};
                *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                *(vertices++) = {Vec3(fx+4.f/6.f, fy+0.f, 0.f), color};
                *(vertices++) = {Vec3(fx+5.f/6.f, fy+r2,  0.f), color};
                *(vertices++) = {Vec3(fx+6.f/6.f, fy+0.f, 0.f), color};
            }
            if(do_sound)
                play_sound(&game_state->sound, SOUND_SPIKES);
        }
        { // Platforms
            RgbaColor color = {30, 30, 30, 255};
            uint S = MIN(platforms_snapshot.size, level->platform_sizes.size);
            for(uint k=0; k<S; k++){
#if DEVMODE
                if(k >= level->platform_sizes.size)
                    break;
#endif
                Vec2 r = platforms_snapshot[k];
                Vec2 s = level->platform_sizes[k];
                Vec3 box[4] = {
                    Vec3(r.x-s.x, r.y-s.y, 0.01f),
                    Vec3(r.x+s.x, r.y-s.y, 0.01f),
                    Vec3(r.x-s.x, r.y+s.y, 0.01f),
                    Vec3(r.x+s.x, r.y+s.y, 0.01f),
                };
                *(vertices++) = {box[0], color};
                *(vertices++) = {box[1], color};
                *(vertices++) = {box[2], color};
                *(vertices++) = {box[2], color};
                *(vertices++) = {box[1], color};
                *(vertices++) = {box[3], color};
            }
        }
        // Water
        if(level->num == 0){
            const float water_equilibrium = 1.8f;
            const float water_amplitude = 0.5f;
            const float water_fluidity = 5.f;
            const float water_antifluidity1 = 5.f;
            const float water_antifluidity2 = 0.99f;
            
            float *water_height = level->water_height;
            float *water_speed = level->water_speed;
            
            int times = (int)roundf(time_step / TIME_STEP * 5.f);
            float ts;
            if(times == 0){
                times = 1;
                ts = time_step;
            }else{
                ts = TIME_STEP;
            }
            for(int k=0; k<times; k++){
                for(int i=0; i<WATER_NODES_1; i++){
                    if(i != 0 && i != WATER_NODES_0){
                        water_speed[i] += water_fluidity * ts * (water_height[i-1] - water_height[i]);
                    }
                    if(i != WATER_NODES_0-1 && i != WATER_NODES_1-1){
                        water_speed[i] += water_fluidity * ts * (water_height[i+1] - water_height[i]);
                    }
                    water_speed[i] += water_antifluidity1 * ts * (water_equilibrium - water_height[i]);
                    
                    water_speed[i] = water_antifluidity2 * water_speed[i];
                }
                for(int i=0; i<WATER_NODES_1; i++){
                    water_height[i] += ts * water_speed[i]; 
                } 
            }
            static float t = 0.f;
            t += time_step;
            
            float x1 = WATER_MIN_X_0;
            float y1 = water_height[0] + 0.02f*(sinf(4.f*x1 - 3.f*t)  + sinf(5.4235f*x1 + 3.f*t));
            for(int i=0; i<WATER_NODES_0-1; i++){
                float x0 = x1;
                float y0 = y1;
                x1 = x0 + water_subdivision_distance;
                y1 = water_height[i+1] + 0.02f*(sinf(4.f*x1 - 3.f*t)  + sinf(5.4235f*x1 + 3.f*t));
                
                Vec3 box[4] = {
                    Vec3(x0, 1.f,  0.f),
                    Vec3(x1, 1.f,  0.f),
                    Vec3(x0, y0,   0.f),
                    Vec3(x1, y1,   0.f),
                };
                *(vertices++) = {box[0], water_color};
                *(vertices++) = {box[1], water_color};
                *(vertices++) = {box[2], water_color};
                *(vertices++) = {box[2], water_color};
                *(vertices++) = {box[1], water_color};
                *(vertices++) = {box[3], water_color};
            }
            
            x1 = WATER_MIN_X_1;
            y1 = water_height[WATER_NODES_0] + 0.02f*(sinf(4.f*x1 - 3.f*t)  + sinf(5.4235f*x1 + 3.f*t));
            for(int i=WATER_NODES_0; i<WATER_NODES_1-1; i++){
                float x0 = x1;
                float y0 = y1;
                x1 = x0 + water_subdivision_distance;
                y1 = water_height[i+1] + 0.02f*(sinf(4.f*x1 - 3.f*t)  + sinf(5.4235f*x1 + 3.f*t));
                
                Vec3 box[4] = {
                    Vec3(x0, 1.f,  0.f),
                    Vec3(x1, 1.f,  0.f),
                    Vec3(x0, y0,   0.f),
                    Vec3(x1, y1,   0.f),
                };
                *(vertices++) = {box[0], water_color};
                *(vertices++) = {box[1], water_color};
                *(vertices++) = {box[2], water_color};
                *(vertices++) = {box[2], water_color};
                *(vertices++) = {box[1], water_color};
                *(vertices++) = {box[3], water_color};
            }
        }
        if(level->num == ArraySize(all_levels)-1){
            static bool first_time = true;
            static Vertex_PCa core_verts[CORE_TRIANGLES * 3];
            const float CORE_RADIUS = 5.f;
            
            if(first_time){
                float angle0 = 0.f;
                u8 red = (u8)(210 + rand()%46);
                u8 green = rand() % (red-100);
                for(int i=0; i<CORE_TRIANGLES; i++){
                    float angle2 = angle0 + 2.f*M_PI/CORE_TRIANGLES;
                    float angle1 = 0.5f*(angle0 + angle2);
                    float r = CORE_RADIUS * 0.f;
                    core_verts[3*i + 0] = {Vec3(CORE_RADIUS*cosf(angle0), CORE_RADIUS*sinf(angle0), 0.f), {red, green, 0, 255}};
                    core_verts[3*i + 2] = {Vec3(r*cosf(angle1), r*sinf(angle1), 0.f), {red, green, 0, 255}};
                    
                    red = (u8)(210 + rand()%46);
                    green = rand()%(red-100);
                    angle0 = angle2;
                    
                    core_verts[3*i + 1] = {Vec3(CORE_RADIUS*cosf(angle0), CORE_RADIUS*sinf(angle0), 0.1f), {red, green, 0, 255}};
                }
                first_time = false;
            }
            
            static float v = 5.f;
            if(game_state->ending_animation_info.started){
                v += 8.f*time_step;
            }else{
                v = 5.f;
            }
            static float t = 0.f;
            t += v*time_step;
            if(t > 2.f*M_PI) t -= 2.f*M_PI;
            
            float c = cosf(t);
            float s = sinf(t);
            
            for(int i=0; i<CORE_TRIANGLES; i++){
                for(int j=0; j<3; j++){
                    float x = core_verts[3*i+j].p.x;
                    float y = core_verts[3*i+j].p.y;
                    *(vertices++) = {Vec3(12.5f + c*x - s*y, 11.5f + s*x + c*y, core_verts[3*i+j].p.z), core_verts[3*i+j].c};
                }
            }
            
            float x0 = 20.f;
            float x1 = 23.f;
            float y0 = player_r.x > 10.f ? fmaxf(16.f, player_r.y + player_size.y) : 16.f;
            float y1 = 17.f;
            RgbaColor color = {178, 0, 0, 255};
            *(vertices++) = {{x0, y0, 0.f}, color};
            *(vertices++) = {{x1, y0, 0.f}, color};
            *(vertices++) = {{x0, y1, 0.f}, color};
            *(vertices++) = {{x0, y1, 0.f}, color};
            *(vertices++) = {{x1, y0, 0.f}, color};
            *(vertices++) = {{x1, y1, 0.f}, color};
        }
        
        buffer->count = (u32)(vertices-o_vertices);
        ggtgl_set_buffer_data(buffer->buffer, o_vertices, buffer->count, GL_DYNAMIC_DRAW);
        end_temp_alloc();
    }
}
