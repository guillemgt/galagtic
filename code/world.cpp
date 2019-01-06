#include "basecode/os.hpp"

#include "world.hpp"
#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

u16 block_info[256];

const char pre_level_txt[] =
#include "world.txt"
;

const float flag_speed = 3.f;

extern Vec2i window_size;

void load_world(GameState *game_state){
    LevelInfo *li = game_state->level_infos;
    
    int w = 0;
    int h = 0;
    int l = 0;
    char last_c = ' ';
    for(int i=1; i<ArrayCount(pre_level_txt)-1; i++){
        char c = pre_level_txt[i];
        if(c == '\n'){
            if(last_c != '\n'){
                h++;
            }else{
                li[l].size = Vec2i(w, h);
                w = 0;
                h = 0;
                l++;
            }
        }else{
            if(h == 0) w++;
        }
        last_c = c;
    }
    game_state->levels_num = l-1;
    
    l = 0;
    li[l].enemies.size = 0;
    li[l].platforms.size = 0;
    int x = 0, y = li[l].size.y-1;
    for(int i=1; i<ArrayCount(pre_level_txt)-2; i++){
        char c = pre_level_txt[i];
        if(c == '\n'){
            l++;
            li[l].enemies.size = 0;
            li[l].platforms.size = 0;
            x = 0;
            y = li[l].size.y-1;
            continue;
        }
        li[l].layout[x][y] = c;
        if(li[l].layout[x][y] == 'P'){
            li[l].layout[x][y] = ' ';
            li[l].start = Vec2(x+0.5f, y+0.5f);
        }else if(li[l].layout[x][y] == 'Q'){
            li[l].layout[x][y] = '.';
            li[l].start = Vec2(x+0.5f, y+0.5f);
        }else if(li[l].layout[x][y] == 'k'){
            li[l].layout[x][y] = ' ';
            li[l].enemies.push(Vec2(x+0.5f, y+0.5f));
        }else if(li[l].layout[x][y] == 'c'){
            li[l].layout[x][y] = ' ';
            li[l].platforms.push(Vec2(x+0.5f, y+0.5f));
        }
        x++;
        if(pre_level_txt[i+1] == '\n'){
            if(x != 0)
                y--;
            x = 0;
            i++;
        }
    }
    /*for(int l=0; l<levels_num; l++){
        int w = level_sizes[l].x;
        int h = level_sizes[l].y-1;
        for(int x=0; x<w; x++){
            level_moving_layouts[l][x][0] = ' ';
            level_layouts[l][x][0] = '#';
            level_moving_layouts[l][x][h] = ' ';
            level_layouts[l][x][h] = '#';
        }
    }*/
    
    block_info[' ']  = BLOCK_IS_TRANSPARENT;
    block_info['#']  = BLOCK_IS_SOLID;
    block_info['^']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_TRANSPARENT;
    block_info['v']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_TRANSPARENT;
    block_info['>']  = BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info['<']  = BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['/']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info['\\'] = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['7']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT;
    block_info['1']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT;
    block_info['U']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['D']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['R']  = BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['L']  = BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['x']  = BLOCK_IS_HARMFUL_UP    | BLOCK_IS_TRANSPARENT | BLOCK_IS_RANDOM;
    block_info['y']  = BLOCK_IS_HARMFUL_DOWN  | BLOCK_IS_TRANSPARENT | BLOCK_IS_RANDOM;
    block_info['z']  = BLOCK_IS_HARMFUL_RIGHT | BLOCK_IS_TRANSPARENT | BLOCK_IS_RANDOM;
    block_info['w']  = BLOCK_IS_HARMFUL_LEFT  | BLOCK_IS_TRANSPARENT | BLOCK_IS_RANDOM;
    block_info['*']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT;
    block_info['+']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['%']  = BLOCK_IS_GOAL | BLOCK_IS_TRANSPARENT;
    block_info['a']  = BLOCK_IS_TRANSPARENT;
    block_info['b']  = BLOCK_IS_TRANSPARENT | BLOCK_IS_SOLID | BLOCK_IS_HARMFUL_CENTER;
    block_info['.']  = BLOCK_IS_TRANSPARENT | BLOCK_HAS_TWO_LAYERS;
    block_info['-']  = BLOCK_IS_TRANSPARENT | BLOCK_STOPS_PLATFORMS;
}

void load_level(GameState *game_state, int num){
    if(num < 0 || num >= game_state->levels_num)
        return;
    
    printf("Loading level #%i/%i\n", num, game_state->levels_num);
    
    Player *player = &game_state->player;
    Level  *level  = &game_state->level;
    
    LevelInfo *level_info = game_state->level_infos + num;
    LaggedLevel *lagged_level = &game_state->next_lagged_level;
    
    level->num = num;
    for(int x=0; x<level_info->size.x; x++){
        for(int y=0; y<level_info->size.y; y++){
            level->layout[x][y] = level_info->layout[x][y];
        }
    }
    
    level->width  = level_info->size.x;
    level->height = level_info->size.y;
    
    level->state = 0;
    
    level_size = Vec2((f32)level->width, (f32)level->height-2.f);
    
    for(int x=0; x<level->width; x++){
        for(int y=0; y<level->height; y++){
            char c = level->layout[x][y];
            if(c == '*' || c == '%' || c == '+'){
                level->goal_r = Vec2(x+0.5f, y+0.5f);
                break;
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
        level->platforms[i] = {level_info->platforms[i], Vec2(2.f, 2.5f), Vec2(2.f, 0.f)};
        lagged_level->platform_sizes[i] = Vec2(2.f, 2.5f);
    }
    
    int y, x = (int)level->goal_r.x;
    for(y=(int)level->goal_r.y; y>=0; y--){
        if(level->layout[x][y] == '#')
            break;
    }
    lagged_level->flag_base_y = (f32)y + 1.f;
    lagged_level->flag_top_y  = level->goal_r.y;
    lagged_level->flag_x      = level->goal_r.x;
    lagged_level->flag_y      = level->goal_r.y;
    level->time_to_load_next_level_max = (lagged_level->flag_top_y - lagged_level->flag_base_y - 0.5f)/flag_speed + time_after_flag_is_down;
    
    reset_player(level_info, player, level);
}


float some_rand(float x, float y){
    return 1.f+0.5f*(sin(54376.f*x)+cos(87343.f*y));
}

void load_level_into_buffer(Level *level, BufferAndCount *buffer, BufferAndCount *t_buffer){
    auto &layout = level->layout;
    uint vertex_count = 0;
    uint t_vertex_count = 0;
    
    {
        for(int x=0; x<level->width; x++){
            for(int y=0; y<level->height; y++){
                char c = layout[x][y];
                if(block_info[c] & BLOCK_IS_TRANSPARENT)
                    t_vertex_count += 6;
                if(c == '#')
                    vertex_count += 12;
                else if(c == '/' || c == '\\' || c == '7' || c == '1')
                    vertex_count += 18;
                else if(block_info[c] & BLOCK_IS_HARMFUL)
                    vertex_count += 9;
                //else if((c == '*' || c == '%') && !player.completed_level)
                //    vertex_count += 6;
                if(block_info[c] & BLOCK_HAS_TWO_LAYERS)
                    vertex_count += 6;
            }
        }
        
        
        start_temp_alloc();
        Vertex_PCa *o_vertices = (Vertex_PCa *)temp_alloc(vertex_count*sizeof(Vertex_PCa));
        Vertex_PCa *vertices = o_vertices;
        
        float fx = 0.f;
        for(int x=0; x<level->width; x++){
            float fy = 0.f;
            for(int y=0; y<level->height; y++){
                char c = layout[x][y];
                switch(c){
                    case '#': {
                        //const RgbaColor color = {(u8)(rand()%127), (u8)(rand()%127), (u8)(rand()%127), 255};
                        const RgbaColor color = basic_color;
                        const float disp = 0.05f;
                        float r0 = disp*((17*x+13*y)%23)/23.f;
                        float r1 = disp*((13*x+17*y)%23)/23.f;
                        float r2 = disp*((19*x+11*y)%23)/23.f;
                        float r3 = disp*((11*x+19*y)%23)/23.f;
                        Vec3 square[4] = {
                            Vec3(fx+0.05f-r0, fy+0.05f-r1, 0.f),
                            Vec3(fx+0.95f+r1, fy+0.05f-r3, 0.f),
                            Vec3(fx+0.05f-r2, fy+0.95f+r0, 0.f),
                            Vec3(fx+0.95f+r3, fy+0.95f+r2, 0.f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                        Vec3 bsquare[4] = {
                            Vec3(fx+0.f-r0, fy+0.f-r1, 0.f),
                            Vec3(fx+1.f+r1, fy+0.f-r3, 0.f),
                            Vec3(fx+0.f-r2, fy+1.f+r0, 0.f),
                            Vec3(fx+1.f+r3, fy+1.f+r2, 0.f),
                        };
                        const RgbaColor bcolor = {40, 50, 40, 255};
                        *(vertices++) = {bsquare[0], bcolor};
                        *(vertices++) = {bsquare[1], bcolor};
                        *(vertices++) = {bsquare[2], bcolor};
                        *(vertices++) = {bsquare[2], bcolor};
                        *(vertices++) = {bsquare[1], bcolor};
                        *(vertices++) = {bsquare[3], bcolor};
                    } break;
                    case '^':
                    case 'U':
                    case 'x':{
                        //const RgbaColor color = {(u8)(rand()%127), (u8)(rand()%127), (u8)(rand()%127), 255};
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
                    } break;
                    case 'v':
                    case 'D':
                    case 'y': {
                        //const RgbaColor color = {(u8)(rand()%127), (u8)(rand()%127), (u8)(rand()%127), 255};
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
                    } break;
                    case '>':
                    case 'R':
                    case 'z': {
                        //const RgbaColor color = {(u8)(rand()%127), (u8)(rand()%127), (u8)(rand()%127), 255};
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
                    case '<':
                    case 'L':
                    case 'w': {
                        //const RgbaColor color = {(u8)(rand()%127), (u8)(rand()%127), (u8)(rand()%127), 255};
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
                    case '\\': {
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
                }
                if(block_info[c] & BLOCK_HAS_TWO_LAYERS){
                    const RgbaColor color = {0, 0, 0, 25};
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
                }
                fy += 1.f;
            }
            fx += 1.f;
        }
        
        buffer->count = (u32)(vertices-o_vertices);
        assert(buffer->count <= vertex_count);
        set_buffer_data_static(buffer->buffer, o_vertices, buffer->count);
        
        end_temp_alloc();
    }
    
    
    if(t_buffer != nullptr){
        start_temp_alloc();
        Vertex_PN *o_t_vertices = (Vertex_PN *)temp_alloc(t_vertex_count*sizeof(Vertex_PN));
        Vertex_PN *t_vertices = o_t_vertices;
        
        float fx = 0.f;
        for(int x=0; x<level->width; x++){
            float fy = 0.f;
            for(int y=0; y<level->height; y++){
                char c = layout[x][y];
                if(block_info[c] & BLOCK_IS_TRANSPARENT){
                    Vec3 t00 = {0.f, 0.f, 6.28f*some_rand((f32)x, (f32)y)};
                    Vec3 t01 = {0.f, 1.f, 6.28f*some_rand((f32)x, (f32)y)};
                    Vec3 t10 = {1.f, 0.f, 6.28f*some_rand((f32)x, (f32)y)};
                    Vec3 t11 = {1.f, 1.f, 6.28f*some_rand((f32)x, (f32)y)};
                    u8 tms = rand()%4;
                    for(u8 i=0; i<tms; i++){
                        Vec3 tmp = t00;
                        t00 = t01;
                        t01 = t11;
                        t11 = t10;
                        t10 = tmp;
                    }
                    *(t_vertices++) = {Vec3(fx+0.f, fy+0.f, 0.2f), t00};
                    *(t_vertices++) = {Vec3(fx+1.f, fy+0.f, 0.2f), t10};
                    *(t_vertices++) = {Vec3(fx+0.f, fy+1.f, 0.2f), t01};
                    *(t_vertices++) = {Vec3(fx+0.f, fy+1.f, 0.2f), t01};
                    *(t_vertices++) = {Vec3(fx+1.f, fy+0.f, 0.2f), t10};
                    *(t_vertices++) = {Vec3(fx+1.f, fy+1.f, 0.2f), t11};
                }
                fy += 1.f;
            }
            fx += 1.f;
        }
        
        t_buffer->count = (u32)(t_vertices-o_t_vertices);
        assert(t_buffer->count <= t_vertex_count);
        set_buffer_data_static(t_buffer->buffer, o_t_vertices, t_buffer->count);
        end_temp_alloc();
    }
    
    shown_level_size = level_size;
    screen_translate = Vec2(-0.5f*level->width, -0.5f*level->height);
}

void load_changing_level_into_buffer(Level *level, BufferAndCount *buffer){
    auto &layout = level->layout;
    uint vertex_count = 0;
    for(int x=0; x<level->width; x++){
        for(int y=0; y<level->height; y++){
            char c = layout[x][y];
            if(c == 'b')
                vertex_count += 6;
            else if(c == 'a')
                vertex_count += 12;
        }
    }
    
    start_temp_alloc();
    Vertex_PCa *o_vertices = (Vertex_PCa *)temp_alloc(vertex_count*sizeof(Vertex_PCa));
    Vertex_PCa *vertices = o_vertices;
    
    float fx = 0.f;
    for(int x=0; x<level->width; x++){
        float fy = 0.f;
        for(int y=0; y<level->height; y++){
            char c = layout[x][y];
            switch(c){
                case 'a': {
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
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    }
                    {
                        const RgbaColor color = {100, 100, 100, 255};
                        Vec3 square[4] = {
                            Vec3(fx+0.05f-r0, fy+0.05f-r1, 0.19f),
                            Vec3(fx+0.95f+r1, fy+0.05f-r3, 0.19f),
                            Vec3(fx+0.05f-r2, fy+0.95f+r0, 0.19f),
                            Vec3(fx+0.95f+r3, fy+0.95f+r2, 0.19f),
                        };
                        *(vertices++) = {square[0], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[2], color};
                        *(vertices++) = {square[1], color};
                        *(vertices++) = {square[3], color};
                    }
                } break;
                case 'b': {
                    const RgbaColor color = {100, 100, 100, 255};
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
            }
            fy += 1.f;
        }
        fx += 1.f;
    }
    
    buffer->count = (u32)(vertices-o_vertices);
    assert(buffer->count <= vertex_count);
    set_buffer_data_static(buffer->buffer, o_vertices, buffer->count);
    
    end_temp_alloc();
}

void load_goal_into_buffer(GameState *game_state, BufferAndCount *buffer){
    const int triangles = 7;
    const int flag_segments = 8;
    const float flag_segments_plus_1 = 9.f;
    const int flag_triangles = 2*flag_segments+1;
    const float flag_segment_length = 0.1f;
    const float flag_height = 0.6f;
    static Vec3 triangle_positions[triangles][3];
    static bool first_time = true;
    static float triangle_periods[triangles][3];
    if(first_time){
        for(int i=0; i<triangles; i++){
            float angle = 2.f*M_PI*(float)rand()/RAND_MAX;
            for(int j=0; j<3; j++){
                float r = 0.15f+0.35f*(float)rand()/RAND_MAX;
                float c = cos(angle);
                float s = sin(angle);
                triangle_positions[i][j] = Vec3(r*c, r*s, -0.1f*(float)rand()/RAND_MAX);
                triangle_periods[i][j] = 0.35f*sqrtf(r);
                angle += 2.f*M_PI/3.f;
            }
        }
        first_time = false;
    }
    static RgbaColor triangle_colors[triangles] = {
        { 155,   0,   0, 255 },
        {   0, 155,   0, 255 },
        {   0,   0, 155, 255 },
        {   0,   0,   0, 255 },
        { 155, 155,   0, 255 },
        { 155,   0, 155, 255 },
        {   0, 155, 155, 255 },
    };
    {
        start_temp_alloc();
        
        auto &enemies_snapshot = game_state->enemies_snapshots[game_state->last_rendered_snapshot];
        auto &platforms_snapshot = game_state->platforms_snapshots[game_state->last_rendered_snapshot];
        u8 level_completed = game_state->completion_snapshots[game_state->last_rendered_snapshot];
        int vertex_count = 3*(flag_triangles) + 6 + 3*triangles*(enemies_snapshot.size) + 6*platforms_snapshot.size;
        
        Vertex_PCa *o_vertices = (Vertex_PCa *)temp_alloc(vertex_count*sizeof(Vertex_PCa));
        Vertex_PCa *vertices = o_vertices;
        
        LaggedLevel *level = &game_state->lagged_level;
        { // We draw the flag
            float flag_base_y = level->flag_base_y;
            
            RgbaColor color;
            if(!level_completed)
                color = {155, 0, 0, 255};
            else if(level->flag_y > level->flag_base_y + 0.5f)
                color = {140, 100, 20, 255};
            else
                color = {0, 100, 0, 255};
            static float t = 0.f;
            t += TIME_STEP;
            
            float height = flag_height;
            float oldheight = height;
            Vec3 r = {level->flag_x - 0.5f*flag_segment_length*flag_segments_plus_1, level->flag_top_y+0.5f*flag_height, 0.f};
            
            {
                const RgbaColor pcolor = {120, 120, 120, 255};
                const float z = 0.1f;
                const float width = 0.1f;
                *(vertices++) = {Vec3(r.x,       r.y,         z), pcolor};
                *(vertices++) = {Vec3(r.x,       flag_base_y, z), pcolor};
                *(vertices++) = {Vec3(r.x+width, r.y,         z), pcolor};
                *(vertices++) = {Vec3(r.x+width, r.y,         z), pcolor};
                *(vertices++) = {Vec3(r.x,       flag_base_y, z), pcolor};
                *(vertices++) = {Vec3(r.x+width, flag_base_y, z), pcolor};
            }
            
            r.y = level->flag_y-0.5f*flag_height;
            if(level_completed){
                level->flag_y -= flag_speed*TIME_STEP;
                if(level->flag_y < level->flag_base_y + 0.5f)
                    level->flag_y = level->flag_base_y + 0.5f;
            }else if(!level_completed){
                level->flag_y += 30.f*TIME_STEP;
                if(level->flag_y > level->flag_top_y)
                    level->flag_y = level->flag_top_y;
            }
            
            Vec3 oldr0 = r, oldr1 = {r.x, r.y+flag_height, r.z};
            for(int i=0; i<flag_segments; i++){
                r.x += flag_segment_length;
                r.y += 0.5f * flag_height / flag_segments_plus_1;
                height -= flag_height / flag_segments_plus_1;
                Vec3 newr0 = {r.x, r.y+0.05f*cos(8.f*r.x+2.f*t), r.z};
                Vec3 newr1 = {r.x, newr0.y+height, r.z};
                *(vertices++) = {oldr0, color};
                *(vertices++) = {oldr1, color};
                *(vertices++) = {newr0, color};
                *(vertices++) = {newr0, color};
                *(vertices++) = {oldr1, color};
                *(vertices++) = {newr1, color};
                oldr0 = newr0;
                oldr1 = newr1;
            }
            r.x += flag_segment_length;
            r.y += 0.5f*flag_height/flag_segments_plus_1 + 0.05f*cos(8.f*r.x+2.f*t);
            *(vertices++) = {oldr0, color};
            *(vertices++) = {oldr1, color};
            *(vertices++) = {r, color};
        }
        {
            RgbaColor color = {0, 0, 0, 255};
            for(uint k=0; k<enemies_snapshot.size; k++){
                Vec2 p = enemies_snapshot[k];
                for(int i=0; i<triangles; i++){
                    for(int j=0; j<3; j++){
                        float t = game_state->time/triangle_periods[i][j];
                        float s = cos(t);
                        *(vertices++) = {Vec3(p.x+s*triangle_positions[i][j].x, p.y+s*triangle_positions[i][j].y, 0.01f), color};
                    }
                }
            }
        }
        {
            RgbaColor color = {0, 0, 0, 255};
            for(uint k=0; k<platforms_snapshot.size; k++){
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
        
        buffer->count = (u32)(vertices-o_vertices);
        set_buffer_data_dynamic(buffer->buffer, o_vertices, buffer->count);
        end_temp_alloc();
    }
}
