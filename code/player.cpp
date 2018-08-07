#include "basecode/os.hpp"

#include "player.hpp"
#include "render.hpp"
#include "world.hpp"
#include "main.hpp"
#include "sound.hpp"

#define MAX_PARTICLES 9000
struct Particle {
    Vec2 r, v;
    RgbaColor color;
    float glitched_life;
};
StaticArray<Particle, MAX_PARTICLES> particles(0);

#define MAX_FPS 1000
Player player_snapshots[MAX_FPS];
int next_free_player_snapshot = 0;
int last_rendered_player_snapshot = 0;
Player rendered_player;

ParticleMessage particle_messages[MAX_FPS];
SoundMessage       sound_messages[MAX_FPS];

const float player_walk_speed = 10.f;
const float player_walk_acceleration = 5.f*60.f;
const float player_jump_speed = 9.6f;
const float player_air_acceleration = 3.f*60.f;
const float player_max_speed_down = -12.f;
const float player_max_speed_down_while_sticking_to_wall = -4.f;
const float time_sticking_to_wall = 4.f/60.f;
const float wall_jump_speed_x = 10.f;
const float gravity = -40.f;
const float after_wall_jump_acceleration = 0.2f*60.f;
const float falling_things_speed = -0.5f;
const Vec2 player_size = Vec2(0.1625f, 0.4f);
const Vec2 player_box_size = Vec2(0.1625f, 0.2f);
const float player_y_offset = 0.f;

StaticArray<Vec2, MAX_ENEMIES> level_enemies[max_levels];
StaticArray<Vec2, MAX_ENEMIES> enemies;
const Vec2 enemies_speed = Vec2(-2.f, 0.f);
const Vec2 enemies_box_size = Vec2(0.2f, 0.2f);

float level_moving_y;

Player player;
GameStats game_stats;
int level_state;

extern f32 TIME_STEP;

extern char **level_layout;

inline float sfloor(float x){ // Strict floor
    float t = floor(x);
    if(t == x)
        return t-1.f;
    else
        return t;
}
inline Vec3 sfloor(Vec3 v){
    return Vec3(sfloor(v.x), sfloor(v.y), sfloor(v.z));
}


void init_messages(){
    for(int i=0; i<MAX_FPS; i++){
        particle_messages[i].time = INFINITY;
        sound_messages[i].time = INFINITY;
    }
}
void add_particle_message(Vec2 r, bool is_death){
    static int next_free_slot = 0;
    particle_messages[next_free_slot].time = player.game_time;
    particle_messages[next_free_slot].r = r;
    particle_messages[next_free_slot].is_death = is_death;
    next_free_slot = (next_free_slot + 1) % MAX_FPS;
    particle_messages[next_free_slot].time = INFINITY;
}
void add_sound_message(u8 sound){
    static int next_free_slot = 0;
    sound_messages[next_free_slot].time = player.game_time;
    sound_messages[next_free_slot].sound = sound;
    next_free_slot = (next_free_slot + 1) % MAX_FPS;
    sound_messages[next_free_slot].time = INFINITY;
}

void process_movement(u8 keys){
    bool moving = false;
    static float time_to_unstick = time_sticking_to_wall;
    static u8 direction_of_wall_jump;
    if(keys & frame_key_left){
        if(direction_of_wall_jump == 2){
            player.v.x -= after_wall_jump_acceleration*TIME_STEP;
        }else if(player.sticking_to_wall == 1){
            time_to_unstick -= TIME_STEP;
            if(time_to_unstick < 0){
                player.sticking_to_wall = 0;
            }
        }else if(player.on_ground){
            if(player.v.x > 0.f) player.v.x = 0.f;
            player.v.x -= player_walk_acceleration*TIME_STEP;
        }else if(player.sticking_to_wall == 0){
            player.v.x -= player_air_acceleration*TIME_STEP;
        }
        if(player.v.x < -player_walk_speed)
            player.v.x = -player_walk_speed;
        moving = true;
    }else if(player.sticking_to_wall == 1){
        time_to_unstick = time_sticking_to_wall;
    }
    if(keys & frame_key_right){
        if(direction_of_wall_jump == 1){
            player.v.x += after_wall_jump_acceleration*TIME_STEP;
        }else if(player.sticking_to_wall == 2){
            time_to_unstick -= TIME_STEP;
            if(time_to_unstick < 0){
                player.sticking_to_wall = 0;
            }
        }else if(player.on_ground){
            if(player.v.x < 0.f) player.v.x = 0.f;
            player.v.x += player_walk_acceleration*TIME_STEP;
        }else if(player.sticking_to_wall == 0){
            player.v.x += player_air_acceleration*TIME_STEP;
        }
        if(player.v.x > +player_walk_speed)
            player.v.x = +player_walk_speed;
        moving = true;
    }else if(player.sticking_to_wall == 2){
        time_to_unstick = time_sticking_to_wall;
    }
    if(!moving){
        if(player.on_ground)
            player.v.x *= 0.5f;
        else if(direction_of_wall_jump != 0)
            player.v.x *= 0.96f;
        else
            player.v.x *= 0.6f;
    }
    
    static bool space_was_up = false;
    
    if(player.jump_height >= 0.f && player.jump_height < 1.5f){
        if(keys & frame_key_jump){
            player.v.y -= gravity * TIME_STEP;
        }
        player.jump_height += player.v.y*TIME_STEP;
    }
    if(keys & frame_key_jump){
        if(player.on_ground){
            player.v.y = player_jump_speed;
            player.on_ground = false;
            player.jump_height = 0.f;
            space_was_up = false;
            add_sound_message(SOUND_JUMP);
        }else if(space_was_up){
            if(player.sticking_to_wall == 1){
                player.v.x = -wall_jump_speed_x;
                player.v.y = player_jump_speed;
                player.jump_height = 0.f;
                space_was_up = false;
                player.sticking_to_wall = 0;
                direction_of_wall_jump = 1;
                add_sound_message(SOUND_JUMP);
            }else if(player.sticking_to_wall == 2){
                player.v.x = wall_jump_speed_x;
                player.v.y = player_jump_speed;
                player.jump_height = 0.f;
                space_was_up = false;
                player.sticking_to_wall = 0;
                direction_of_wall_jump = 2;
                add_sound_message(SOUND_JUMP);
            }
        }
    }else if(player.sticking_to_wall != 0){
        space_was_up = true;
    }
    
    float time_left = TIME_STEP;
    u8 can_skip = 0;
    int counter = 0;
    bool kill = false;
    while(time_left > 0.f){
        counter++;
        if(counter == 10){
            kill = true;
            break;
        }
        f32 min_time = time_left;
        u8 direction = 255;
        
        if(player.v.x > 0.f){
            if(!(can_skip & (1 << 0))){
                float r = player.r.x + player_size.x;
                float fr = ceil(r);
                float v = player.v.x;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 0;
                }
            }
        }else if(player.v.x < 0.f){
            if(!(can_skip & (1 << 1))){
                float r = player.r.x - player_size.x;
                float fr = floor(r);
                float v = player.v.x;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 1;
                }
            }
        }
        
        if(player.v.y > 0.f){
            if(!(can_skip & (1 << 2))){
                float r = player.r.y + player_size.y;
                float fr = ceil(r);
                float v = player.v.y;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 2;
                }
            }
        }else if(player.v.y < 0.f){
            if(!(can_skip & (1 << 3))){
                float r = player.r.y - player_size.y;
                float fr = floor(r);
                float v = player.v.y;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 3;
                }
            }
        }
        
        if(player.v.y > falling_things_speed){
            if(!(can_skip & (1 << 4))){
                float r = player.r.y + player_size.y - level_moving_y;
                float fr = ceil(r);
                float v = player.v.y - falling_things_speed;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 4;
                }
            }
        }else if(player.v.y < falling_things_speed){
            if(!(can_skip & (1 << 5))){
                float r = player.r.y - player_size.y - level_moving_y;
                float fr = floor(r);
                float v = player.v.y - falling_things_speed;
                f32 t = (fr-r)/v;
                if(t < min_time){
                    min_time = t;
                    direction = 5;
                }
            }
        }
        
        player.r += player.v * min_time;
        level_moving_y += min_time*falling_things_speed;
        if(direction == 0){
            int x = (int)floor(player.r.x) + 1;
            if(x >= level_width)
                player.v.x = 0.f;
            else{
                {
                    int low_y = (int)floor(player.r.y-player_size.y);
                    int high_y = (int)sfloor(player.r.y+player_size.y);
                    for(int y=low_y; y<=high_y; y++){
                        if(block_info[level_layout[x][y]] & BLOCK_IS_SOLID){
                            player.v.x = 0;
                            break;
                        }
                    }
                }
                {
                    int low_y = (int)floor(player.r.y-player_size.y-level_moving_y);
                    int high_y = (int)sfloor(player.r.y+player_size.y-level_moving_y);
                    for(int y=low_y; y<=high_y; y++){
                        if(block_info[level_moving_layout[x][y]] & BLOCK_IS_SOLID){
                            player.v.x = 0;
                            break;
                        }
                    }
                }
            }
            if(player.v.x != 0.f){
                can_skip |= (1 << 0);
            }else{
                player.sticking_to_wall = 1;
                player.r.x = ceil(player.r.x) - player_size.x;
            }
        }else if(direction == 1){
            int x = (int)floor(player.r.x) - 1;
            if(x < 0)
                player.v.x = 0.f;
            else{
                {
                    int low_y = (int)floor(player.r.y-player_size.y);
                    int high_y = (int)sfloor(player.r.y+player_size.y);
                    for(int y=low_y; y<=high_y; y++){
                        if(block_info[level_layout[x][y]] & BLOCK_IS_SOLID){
                            player.v.x = 0;
                            break;
                        }
                    }
                }
                {
                    int low_y = (int)floor(player.r.y-player_size.y-level_moving_y);
                    int high_y = (int)sfloor(player.r.y+player_size.y-level_moving_y);
                    for(int y=low_y; y<=high_y; y++){
                        if(block_info[level_moving_layout[x][y]] & BLOCK_IS_SOLID){
                            player.v.x = 0;
                            break;
                        }
                    }
                }
            }
            if(player.v.x != 0.f){
                can_skip |= (1 << 1);
            }else{
                player.sticking_to_wall = 2;
                player.r.x = floor(player.r.x) + player_size.x;
            }
        }else if(direction == 2){
            int y = (int)floor(player.r.y) + 1;
            if(y >= level_height){
                player.v.y = 0.f;
                player.jump_height = -1.f;
            }else{
                int low_x = (int)floor(player.r.x-player_size.x);
                int high_x = (int)sfloor(player.r.x+player_size.x);
                for(int x=low_x; x<=high_x; x++){
                    if(block_info[level_layout[x][y]] & BLOCK_IS_SOLID){
                        player.v.y = 0;
                        player.jump_height = -1.f;
                        break;
                    }
                }
            }
            if(player.v.y != 0.f){
                can_skip |= (1 << 2);
            }else{
                //player.r.y = ceil(player.r.y) - player_size.y;
            }
        }else if(direction == 3){
            int y = (int)floor(player.r.y) - 1;
            if(y < 0)
                player.v.y = 0.f;
            else{
                int low_x = (int)floor(player.r.x-player_size.x);
                int high_x = (int)sfloor(player.r.x+player_size.x);
                for(int x=low_x; x<=high_x; x++){
                    if(block_info[level_layout[x][y]] & BLOCK_IS_SOLID){
                        player.v.y = 0;
                        break;
                    }
                }
            }
            if(player.v.y != 0.f){
                can_skip |= (1 << 3);
                player.on_ground = false;
            }else{
                player.on_ground = true;
                player.jump_height = -1.f;
                //player.r.y = floor(player.r.y) + player_size.y;
            }
        }else if(direction == 4){
            int y = (int)floor(player.r.y-level_moving_y) + 1;
            if(y < level_height){
                int low_x = (int)floor(player.r.x-player_size.x);
                int high_x = (int)sfloor(player.r.x+player_size.x);
                for(int x=low_x; x<=high_x; x++){
                    if(block_info[level_moving_layout[x][y]] & BLOCK_IS_SOLID){
                        player.v.y = falling_things_speed;
                        player.jump_height = -1.f;
                        break;
                    }
                }
            }
            if(player.v.y != falling_things_speed){
                can_skip |= (1 << 4);
            }
        }else if(direction == 5){
            const float next_speed = 0.5f*falling_things_speed;
            int y = (int)floor(player.r.y-level_moving_y) - 1;
            if(y >= 0){
                int low_x  = (int) floor(player.r.x-player_size.x);
                int high_x = (int)sfloor(player.r.x+player_size.x);
                for(int x=low_x; x<=high_x; x++){
                    if(block_info[level_moving_layout[x][y]] & BLOCK_IS_SOLID){
                        player.v.y = next_speed;
                        break;
                    }
                }
            }
            if(player.v.y != next_speed){
                can_skip |= (1 << 5);
                player.on_ground = false;
            }else{
                player.on_ground = true;
                player.jump_height = -1.f;
            }
        }
        if(direction >= 2 && player.sticking_to_wall){
            bool unstick = true;
            int x = -1;
            if(player.sticking_to_wall == 1){
                x = (int)floor(player.r.x)+1;
            }else if(player.sticking_to_wall == 2){
                x = (int)floor(player.r.x)-1;
            }
            if(x < 0 || x >= level_width)
                unstick = false;
            else{
                {
                    int y = (int)floor(player.r.y);
                    if(block_info[level_layout[x][y]] & BLOCK_IS_SOLID)
                        unstick = false;
                }{
                    int y = (int)floor(player.r.y-level_moving_y);
                    
                    if(block_info[level_moving_layout[x][y]] & BLOCK_IS_SOLID)
                        unstick = false;
                }
            }
            if(unstick)
                player.sticking_to_wall = 0;
        }
        time_left -= min_time;
        if(min_time > 0.f){
            can_skip = 0;
        }
    }
    if(player.on_ground)
        player.sticking_to_wall = 0;
    if(player.sticking_to_wall == 1 && player.v.x < 0.f)
        player.sticking_to_wall = 0;
    if(player.sticking_to_wall == 2 && player.v.x > 0.f)
        player.sticking_to_wall = 0;
    
    player.v.y += gravity * TIME_STEP;
    if(player.sticking_to_wall){
        if(player.v.y < player_max_speed_down_while_sticking_to_wall) player.v.y = player_max_speed_down_while_sticking_to_wall;
    }else{
        if(player.v.y < player_max_speed_down) player.v.y = player_max_speed_down;
    }
    if(player.v.y <= 0.f){
        direction_of_wall_jump = 0;
    }
    
    for(int k=0; k<enemies.size; k++){
        enemies[k] += TIME_STEP*enemies_speed;
        if(enemies[k].x < 0.5f) enemies[k].x += level_size.x-1.f;
    }
    
    if(!kill){
        float f_low_x = player.r.x-player_box_size.x;
        int low_x = (int)floor(f_low_x);
        float f_high_x = player.r.x+player_box_size.x;
        int high_x = (int)sfloor(f_high_x);
        float f_low_y = player.r.y-player_box_size.y;
        int low_y = (int)floor(f_low_y);
        float f_high_y = player.r.y+player_box_size.y;
        int high_y = (int)sfloor(f_high_y);
        for(int x=low_x; x<=high_x; x++){
            for(int y=low_y; y<=high_y; y++){
                char block = level_layout[x][y];
                u8 flags = block_info[block];
                if(flags & BLOCK_IS_GOAL){
                    if(player.load_next_level_in < 0.f){
                        add_particle_message(Vec2((float)x+0.5f, (float)y+0.5f), false);
                        player.load_next_level_in = 1.5f;
                        player.completed_level = true;
                        should_update_level = true;
                        add_sound_message(SOUND_WIN);
                        if(block == '%')
                            game_mode = GAME_MODE_START;
                    }
                }else if(flags & BLOCK_IS_HARMFUL){
                    switch(block){
                        case '^':
                        case 'U':
                        case 'x': {
                            if(f_low_y < y + 0.5f){
                                kill = true;
                                goto out_of_for;
                            }
                        } break;
                        case 'v':
                        case 'D':
                        case 'y': {
                            if(f_high_y > y + 0.5f){
                                kill = true;
                                goto out_of_for;
                            }
                        } break;
                        case '>':
                        case 'R':
                        case 'z': {
                            if(f_low_x < x + 0.5f){
                                kill = true;
                                goto out_of_for;
                            }
                        } break;
                        case '<':
                        case 'L':
                        case 'w': {
                            if(f_high_x > x + 0.5f){
                                kill = true;
                                goto out_of_for;
                            }
                        } break;
                    }
                }
            }
        }
        
        for(int k=0; k<enemies.size; k++){
            Vec2 p = player.r - enemies[k];
            const Vec2 m = player_box_size + enemies_box_size;
            if(-m <= p && p <= m){
                kill = true;
                goto out_of_for;
            }
        }
    }
out_of_for:
    if(kill){
        player.v = 0;
        add_particle_message(player.r, true);
        player.r = player.safe_r;
        level_moving_y = 0.f;
        player.level_time = level_state ? 1.f : 0.f;
        add_sound_message(SOUND_DEATH);
        if(player.completed_level) player.cancel_next_level_in = 2.f;
        player.jump_height = -1.f;
        player.completed_level = false;
        player.lives--;
        
        
        enemies.size = level_enemies[current_level].size;
        for(int i=0; i<enemies.size; i++)
            enemies[i] = level_enemies[current_level][i];
    }
    
    player.level_time += TIME_STEP;
    if(player.level_time >= 2.f){
        player.level_time -= 2.f;
        for(int x=0; x<level_width; x++){
            for(int y=0; y<level_height; y++){
                if(level_layout[x][y] == 'a')
                    level_layout[x][y] = 'b';
                else if(level_layout[x][y] == 'b')
                    level_layout[x][y] = 'a';
            }
        }
        level_state = 1-level_state;
        should_update_level = true;
    }
    
    if(player.cancel_next_level_in >= 0.f){
        player.cancel_next_level_in -= TIME_STEP;
        if(player.cancel_next_level_in < 0.f){
            player.load_next_level_in = -1.f;
        }
    }else if(player.load_next_level_in >= 0.f){
        player.load_next_level_in -= TIME_STEP;
        if(player.load_next_level_in < 0.f){
            load_level(current_level+1);
        }
    }
    
    player_snapshots[next_free_player_snapshot] = player;
    next_free_player_snapshot = (next_free_player_snapshot+1) % MAX_FPS;
}

void process_messages(float time){
    { // Sound
        static int last_message = 0;
        while(sound_messages[last_message].time < time-sound_lag_time){
            play_sound(sound_messages[last_message].sound);
            last_message = (last_message+1) % MAX_FPS;
        }
    }
    
    { // Particles
        static int last_message = 0;
        while(particle_messages[last_message].time < time-particle_lag_time){
            ParticleMessage message = particle_messages[last_message];
            last_message = (last_message+1) % MAX_FPS;
            float fx = message.r.x;
            float fy = message.r.y;
            
            if(message.is_death){
                int s = particles.size;
                particles.size += 12*32;
                assert(particles.size <= MAX_PARTICLES);
                for(int px=-6; px<6; px++){
                    for(int py=-16; py<16; py++){
                        Vec2 speed = (1.f + 7.f*rand()/RAND_MAX)*(Vec2)Angle(2.f*M_PI*rand()/RAND_MAX);
                        particles[s++] = {Vec2(fx+px/30.f, fy+py/30.f), speed, {60, 60, 60, 255}, 0};
                    }
                }
            }else{
                int s = particles.size;
                particles.size += 12*12;
                assert(particles.size <= MAX_PARTICLES);
                static RgbaColor triangle_colors[7] = {
                    { 155, 0, 0, 255 },
                    { 0, 155, 0, 255 },
                    { 0, 0, 155, 255 },
                    { 0, 0, 0, 255 },
                    { 155, 155, 0, 255 },
                    { 155, 0, 155, 255 },
                    { 0, 155, 155, 255 },
                };
                for(int px=-6; px<6; px++){
                    for(int py=-6; py<6; py++){
                        int r = rand();
                        Vec2 speed = (1.f + 7.f*r/RAND_MAX)*(Vec2)Angle(2.f*M_PI*rand()/RAND_MAX);
                        particles[s++] = {Vec2(fx+px/20.f, fy+py/20.f), speed, triangle_colors[r&7], 0};
                    }
                }
            }
        }
        for(int i=particles.size-1; i>=0; i--){
            Particle *particle = &particles[i];
            if(particle->r.y < 1.f || particle->r.x < 1.f || particle->r.x > level_size.x-1.f){
                particles.remove(i);
                continue;
            }
            if(particle->glitched_life > 0.f){
                particle->glitched_life -= TIME_STEP;
                if(particle->glitched_life < 0.f){
                    particles.remove(i);
                    continue;
                }
                if(rand()%2){
                    particle->v.y -= gravity*TIME_STEP;
                    particle->r -= particle->v*TIME_STEP;
                }else{
                    particle->r += particle->v*TIME_STEP;
                    particle->v.y += gravity*TIME_STEP;
                }
            }else{
                particle->r += particle->v*TIME_STEP;
                particle->v.y += gravity*TIME_STEP;
                if(rand()%1000 == 0){
                    particle->glitched_life = 1.f;
                }
            }
        }
    }
    
    // Level
}

void load_player_into_buffer(BufferAndCount *buffer){
    Vertex_PT o_vertices[6];
    Vertex_PT *vertices = o_vertices;
    
    int next_candidate = (last_rendered_player_snapshot + 1) % MAX_FPS;
    while(player_snapshots[next_candidate].time < player.time-visual_lag_time){
        last_rendered_player_snapshot = next_candidate;
        next_candidate = (next_candidate + 1) % MAX_FPS;
    }
    rendered_player = player_snapshots[last_rendered_player_snapshot];
    Player player = rendered_player;
    if(player.load_next_level_in >= 0.f && player.load_next_level_in < TIME_STEP){
        should_new_level = true;
    }
    Vec2 r = player.r+Vec2(0.f, player_y_offset);
    const float texture_size = 256.f;
    const float outer_picture_size = 32.f;
    const float inner_picture_size = 30.f;
    const float picture_margin = 1.f;
    static int current_frame_x = 0;
    static int current_frame_y = 0;
    static int time_in_current_frame = 0;
    time_in_current_frame++;
    bool flip = false;
    if(player.v.x < 0.f)
        flip = true;
    if(player.sticking_to_wall != 0){
        current_frame_y = 4;
        current_frame_x = 0;
        flip = (player.sticking_to_wall == 1);
    }else if(!player.on_ground){
        if(fabs(player.v.x) > 0.3f)
            current_frame_y = 6;
        else{
            current_frame_y = 5;
        }
        float factor = gravity*TIME_STEP;
        if(player.v.y > 3.f+factor){
            current_frame_x = 0;
        }else if(player.v.y > 1.5f+factor){
            current_frame_x = 1;
        }else if(player.v.y > -1.5f+factor){
            current_frame_x = 2;
        }else if(player.v.y > -3.f+factor){
            current_frame_x = 3;
        }else{
            current_frame_x = 4;
        }
    }else if(fabs(player.v.x) > 0.1f){
        current_frame_y = 7;
        if(time_in_current_frame >= 5){
            time_in_current_frame = 0;
            current_frame_x = current_frame_x%6 + 1;
            if(current_frame_x == 1 || current_frame_x == 4){
                static u8 odd = 0;
                odd++;
                if(odd)
                    add_sound_message(SOUND_WALK_0);
                else
                    add_sound_message(SOUND_WALK_1);
            }
        }
    }else{
        current_frame_y = 7;
        current_frame_x = 0;
    }
    Vec2 t00, t10, t01, t11;
    if(flip){
        t10 = Vec2(current_frame_x*outer_picture_size+picture_margin, current_frame_y*outer_picture_size+picture_margin)/texture_size;
        t00 = t10+Vec2(inner_picture_size/texture_size, 0.f);
        t11 = t10+Vec2(0.f, inner_picture_size/texture_size);
        t01 = t00+Vec2(0.f, inner_picture_size/texture_size);
    }else{
        t00 = Vec2(current_frame_x*outer_picture_size+picture_margin, current_frame_y*outer_picture_size+picture_margin)/texture_size;
        t10 = t00+Vec2(inner_picture_size/texture_size, 0.f);
        t01 = t00+Vec2(0.f, inner_picture_size/texture_size);
        t11 = t10+Vec2(0.f, inner_picture_size/texture_size);
    }
    *(vertices++) = {Vec3(r.x-0.4f, r.y-0.4f, 0.1f), t00};
    *(vertices++) = {Vec3(r.x+0.4f, r.y-0.4f, 0.1f), t10};
    *(vertices++) = {Vec3(r.x-0.4f, r.y+0.4f, 0.1f), t01};
    *(vertices++) = {Vec3(r.x-0.4f, r.y+0.4f, 0.1f), t01};
    *(vertices++) = {Vec3(r.x+0.4f, r.y-0.4f, 0.1f), t10};
    *(vertices++) = {Vec3(r.x+0.4f, r.y+0.4f, 0.1f), t11};
    
    buffer->count = (u32)(vertices-o_vertices);
    set_buffer_data_dynamic(buffer->buffer, o_vertices, buffer->count);
}

void load_particles_into_buffer(BufferAndCount *buffer){
    int vertex_count = 6*particles.size;
    if(vertex_count == 0){
        buffer->count = 0;
        return;
    }
    
    start_temp_alloc();
    Vertex_PCa *o_vertices = (Vertex_PCa *)temp_alloc(vertex_count*sizeof(Vertex_PCa));
    Vertex_PCa *vertices = o_vertices;
    
    for(int i=0; i<particles.size; i++){
        const float size = 1.2f/30.f;
        Vec2 p00 = particles[i].r + Vec2(-size, 0.f);
        Vec2 p01 = particles[i].r + Vec2(0.f, -size);
        Vec2 p10 = particles[i].r + Vec2(0.f, +size);
        Vec2 p11 = particles[i].r + Vec2(+size, 0.f);
        RgbaColor color = particles[i].color;
        *(vertices++) = {Vec3(p00.x, p00.y, 0.f), color};
        *(vertices++) = {Vec3(p10.x, p10.y, 0.f), color};
        *(vertices++) = {Vec3(p01.x, p01.y, 0.f), color};
        *(vertices++) = {Vec3(p01.x, p01.y, 0.f), color};
        *(vertices++) = {Vec3(p10.x, p10.y, 0.f), color};
        *(vertices++) = {Vec3(p11.x, p11.y, 0.f), color};
    }
    
    u32 buffer_count = (u32)(vertices-o_vertices);
    buffer->count = buffer_count;
    if(buffer_count > 0)
        set_buffer_data_dynamic(buffer->buffer, o_vertices, buffer_count);
    end_temp_alloc();
}
