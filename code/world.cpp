#include "basecode/os.hpp"

#include "world.hpp"
#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

int level_width;
int level_height;
u16 block_info[256];
Vec2 goal_position;

const char pre_level_txt[] =
#include "world.txt"
;
char **level_layout;
char **level_moving_layout;
char level_layouts[max_levels][max_level_width][max_level_height];
char level_moving_layouts[max_levels][max_level_width][max_level_height];
Vec2i level_sizes[max_levels];
Vec2i level_starts[max_levels];
static char *_level_layout_rows[max_level_width];
static char *_level_moving_layout_rows[max_level_width];
int levels_num;
int current_level;

extern Vec2i window_size;

#define MAX_MODEL_TRIANGLES 104
typedef StaticArray<Vertex_PT, 3*MAX_MODEL_TRIANGLES> Model;
void load_model(const char *filename, Model &wall_model){
    OsFile fp = open_game_file(filename, "rb");
    if(fp != nullptr){
        read_file(&wall_model.size, sizeof(u32), 1, fp);
        assert(wall_model.size <= MAX_MODEL_TRIANGLES);
        
        Vec3  positions[3*MAX_MODEL_TRIANGLES];
        Vec3    normals[MAX_MODEL_TRIANGLES];
        Vec2 tex_coords[3*MAX_MODEL_TRIANGLES];
        read_file(positions, sizeof(Vec3), 3*wall_model.size, fp);
        read_file(tex_coords, sizeof(Vec2), 3*wall_model.size, fp);
        read_file(normals, sizeof(Vec3), wall_model.size, fp);
        close_file(fp);
        
        int triangles = wall_model.size;
        wall_model.size *= 3;
        for(int i=0, j=0; i<triangles; i++){
            wall_model[j] = {positions[j], tex_coords[j]};
            j++;
            wall_model[j] = {positions[j], tex_coords[j]};
            j++;
            wall_model[j] = {positions[j], tex_coords[j]};
            j++;
        }
    }else{
        printf("Couldn't load model '%s'\n", filename);
    }
}

void load_world(){
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
                level_sizes[l] = Vec2i(w, h);
                w = 0;
                h = 0;
                l++;
            }
        }else{
            if(h == 0) w++;
        }
        last_c = c;
    }
    levels_num = l;
    
    l = 0;
    level_enemies[l].size = 0;
    int x = 0, y = level_sizes[l].y-1;
    for(int i=1; i<ArrayCount(pre_level_txt)-2; i++){
        char c = pre_level_txt[i];
        if(c == '\n'){
            l++;
            level_enemies[l].size = 0;
            x = 0;
            y = level_sizes[l].y-1;
            continue;
        }
        level_moving_layouts[l][x][y] = ' ';
        level_layouts[l][x][y] = c;
        if(level_layouts[l][x][y] == 'P'){
            level_layouts[l][x][y] = ' ';
            level_starts[l] = Vec2i(x, y);
        }else if(level_layouts[l][x][y] == 'Q'){
            level_layouts[l][x][y] = '.';
            level_starts[l] = Vec2i(x, y);
        }else if(level_layouts[l][x][y] == 'e'){
            level_moving_layouts[l][x][y] = '#';
            level_layouts[l][x][y] = ' ';
        }else if(level_layouts[l][x][y] == 'E'){
            level_moving_layouts[l][x][y] = '#';
            level_layouts[l][x][y] = '.';
        }else if(level_layouts[l][x][y] == '%'){
            level_moving_layouts[l][x][y] = '#';
        }else if(level_layouts[l][x][y] == 'r'){
            const char harmful_blocks[] = {'x','y','z','w'};
            level_layouts[l][x][y] = harmful_blocks[rand()%4];
        }else if(level_layouts[l][x][y] == 'k'){
            level_layouts[l][x][y] = ' ';
            level_enemies[l].push(Vec2(x+0.5f, y+0.5f));
        }else if(level_layouts[l][x][y] == 'c'){
            level_layouts[l][x][y] = ' ';
            level_platforms[l].push(Vec2(x+0.5f, y+0.5f));
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

void load_level(int num){
    if(num < 0)
        return;
    
    current_level = num;
    level_layout = &_level_layout_rows[0];
    for(int i=0; i<level_sizes[num].x; i++){
        _level_layout_rows[i] = level_layouts[num][i];
    }
    level_moving_layout = &_level_moving_layout_rows[0];
    for(int i=0; i<level_sizes[num].x; i++){
        _level_moving_layout_rows[i] = level_moving_layouts[num][i];
    }
    player.r = Vec2(level_starts[num].x+0.5f, level_starts[num].y+0.5f);
    player.safe_r = player.r;
    level_width = level_sizes[num].x;
    level_height = level_sizes[num].y;
    
    player.v = Vec2(0.f);
    player.completed_level = false;
    player.sticking_to_wall = 0;
    player.load_next_level_in = -1.f;
    player.level_time = 0.f;
    player.jump_height = -1.f;
    player.cancel_next_level_in = -1.f;
    should_update_level = true;
    player.level_moving_y = 0.f;
    level_state = 0;
    level_size = Vec2(level_width, level_height-2.f);
    
    for(int x=0; x<level_width; x++){
        for(int y=0; y<level_height; y++){
            char c = level_layout[x][y];
            if(c == '*' || c == '%' || c == '+'){
                goal_position = Vec2(x+0.5f, y+0.5f);
                break;
            }
        }
    }
    
    enemies.size = level_enemies[num].size;
    for(int i=0; i<enemies.size; i++)
        enemies[i] = level_enemies[num][i];
    
    platforms.size = level_platforms[num].size;
    for(int i=0; i<platforms.size; i++)
        platforms[i] = {level_platforms[num][i], Vec2(2.f, 2.5f), Vec2(1.f, 0.f)};
}


float some_rand(float x, float y){
    return 1.f+0.5f*(sin(54376.f*x)+cos(87343.f*y));
}

void load_level_into_buffer(BufferAndCount *buffer, BufferAndCount *t_buffer){
    char **layout = t_buffer == nullptr ? level_moving_layout : level_layout;
    int vertex_count = 0;
    int t_vertex_count = 0;
    for(int x=0; x<level_width; x++){
        for(int y=0; y<level_height; y++){
            char c = layout[x][y];
            if(block_info[c] & BLOCK_IS_TRANSPARENT)
                t_vertex_count += 6;
            if(c == '#')
                vertex_count += 6;
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
    for(int x=0; x<level_width; x++){
        float fy = 0.f;
        for(int y=0; y<level_height; y++){
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
                case '^':
                case 'U':
                case 'x':{
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
    
    
    if(t_buffer != nullptr){
        start_temp_alloc();
        Vertex_PN *o_t_vertices = (Vertex_PN *)temp_alloc(t_vertex_count*sizeof(Vertex_PN));
        Vertex_PN *t_vertices = o_t_vertices;
        
        float fx = 0.f;
        for(int x=0; x<level_width; x++){
            float fy = 0.f;
            for(int y=0; y<level_height; y++){
                char c = layout[x][y];
                if(block_info[c] & BLOCK_IS_TRANSPARENT){
                    Vec3 t00 = {0.f, 0.f, 6.28f*some_rand(x, y)};
                    Vec3 t01 = {0.f, 1.f, 6.28f*some_rand(x, y)};
                    Vec3 t10 = {1.f, 0.f, 6.28f*some_rand(x, y)};
                    Vec3 t11 = {1.f, 1.f, 6.28f*some_rand(x, y)};
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
    screen_translate = Vec2(-0.5f*level_width, -0.5f*level_height);
}

void load_changing_level_into_buffer(BufferAndCount *buffer){
    char **layout = level_layout;
    int vertex_count = 0;
    for(int x=0; x<level_width; x++){
        for(int y=0; y<level_height; y++){
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
    for(int x=0; x<level_width; x++){
        float fy = 0.f;
        for(int y=0; y<level_height; y++){
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

void load_goal_into_buffer(BufferAndCount *buffer, BufferAndCount *loading_buffer){
    const int triangles = 7;
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
    if(!rendered_player.completed_level || enemies.size > 0 || platforms.size){
        StaticArray<Vec2, MAX_ENEMIES> &enemies_snapshot = enemies_snapshots[last_rendered_player_snapshot];
        StaticArray<Vec2, MAX_PLATFORMS> &platforms_snapshot = platforms_snapshots[last_rendered_player_snapshot];
        int vertex_count = 3*triangles*(1+enemies_snapshot.size) + 6*platforms_snapshot.size;
        
        Vertex_PCa o_vertices[vertex_count];
        Vertex_PCa *vertices = o_vertices;
        
        for(int i=0; i<triangles; i++){
            RgbaColor color = triangle_colors[i];
            for(int j=0; j<3; j++){
                float t = player.time/triangle_periods[i][j];
                float s = cos(t);
                *(vertices++) = {Vec3(goal_position.x+s*triangle_positions[i][j].x, goal_position.y+s*triangle_positions[i][j].y, triangle_positions[i][j].z), color};
            }
        }
        {
            RgbaColor color = {0, 0, 0, 255};
            for(int k=0; k<enemies_snapshot.size; k++){
                Vec2 p = enemies_snapshot[k];
                for(int i=0; i<triangles; i++){
                    for(int j=0; j<3; j++){
                        float t = player.time/triangle_periods[i][j];
                        float s = cos(t);
                        *(vertices++) = {Vec3(p.x+s*triangle_positions[i][j].x, p.y+s*triangle_positions[i][j].y, 0.01f), color};
                    }
                }
            }
        }
        {
            RgbaColor color = {0, 0, 0, 255};
            for(int k=0; k<platforms_snapshot.size; k++){
                Vec2 r = platforms_snapshot[k];
                Vec2 s = platforms[k].size;
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
    }else{
        buffer->count = 0;
    }
    if(rendered_player.completed_level || rendered_player.cancel_next_level_in > 0.f){
        const int vertex_count = 6*5;
        
        Vertex_PCa o_vertices[vertex_count];
        Vertex_PCa *vertices = o_vertices;
        
        RgbaColor color = rendered_player.completed_level ? RgbaColor{255, 255, 255, 180} : RgbaColor{255, 50, 50, (u8)(MIN(rendered_player.cancel_next_level_in, 1.f)*180)};
        
        float t = (load_next_level_in_max-rendered_player.load_next_level_in)/load_next_level_in_max;
        Vec3 box[3][4];
        float y_range = 0.5f*screen_portion_of_ui_bottom*actual_screen_ratio;
        //float half_y_range = 0.5f*y_range;
        float y_sp = 0.5f*y_range;
        float x_sp = 0.5f*y_range;
        float x_range = 1.f-y_range;
        float dists[] = {0.3f*y_range, 0.35f*y_range, 0.4f*y_range};
        //float dists[] = {0.6f*y_range, 0.7f*y_range, 0.8f*y_range};
        const Vec3 line[3][2] = {
            { Vec3(x_sp, y_sp, -1.f), Vec3(x_sp+t*x_range, y_sp, -0.3f) },
            { Vec3(x_sp, y_sp, -1.f), Vec3(x_sp+x_range, y_sp, -0.3f) },
            { Vec3(x_sp, y_sp, -1.f), Vec3(x_sp+x_range, y_sp, -0.3f) },
        };
        for(int i=0; i<3; i++){
            box[i][0] = line[i][0] + Vec3(-dists[i], -dists[i], 0.f);
            box[i][1] = line[i][0] + Vec3(-dists[i], +dists[i], 0.f);
            box[i][2] = line[i][1] + Vec3(+dists[i], -dists[i], 0.f);
            box[i][3] = line[i][1] + Vec3(+dists[i], +dists[i], 0.f);
        }
        
        *(vertices++) = {box[0][0], color};
        *(vertices++) = {box[0][1], color};
        *(vertices++) = {box[0][2], color};
        *(vertices++) = {box[0][2], color};
        *(vertices++) = {box[0][1], color};
        *(vertices++) = {box[0][3], color};
        
        *(vertices++) = {box[1][1], color};
        *(vertices++) = {box[1][0], color};
        *(vertices++) = {box[2][1], color};
        *(vertices++) = {box[2][1], color};
        *(vertices++) = {box[1][0], color};
        *(vertices++) = {box[2][0], color};
        
        *(vertices++) = {box[1][2], color};
        *(vertices++) = {box[1][0], color};
        *(vertices++) = {box[2][2], color};
        *(vertices++) = {box[2][2], color};
        *(vertices++) = {box[1][0], color};
        *(vertices++) = {box[2][0], color};
        
        *(vertices++) = {box[1][1], color};
        *(vertices++) = {box[1][3], color};
        *(vertices++) = {box[2][1], color};
        *(vertices++) = {box[2][1], color};
        *(vertices++) = {box[1][3], color};
        *(vertices++) = {box[2][3], color};
        
        *(vertices++) = {box[1][2], color};
        *(vertices++) = {box[1][3], color};
        *(vertices++) = {box[2][2], color};
        *(vertices++) = {box[2][2], color};
        *(vertices++) = {box[1][3], color};
        *(vertices++) = {box[2][3], color};
        
        loading_buffer->count = (u32)(vertices-o_vertices);
        set_buffer_data_dynamic(loading_buffer->buffer, o_vertices, loading_buffer->count);
    }else{
        loading_buffer->count = 0.f;
    }
}
