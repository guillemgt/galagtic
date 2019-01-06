#include "basecode/os.hpp"

#include "player.hpp"
#include "render.hpp"
#include "world.hpp"
#include "main.hpp"
#include "sound.hpp"

// Lateral movement
const float player_walk_speed = 10.f;
const float player_walk_acceleration = 5.f*60.f;
const float player_air_acceleration = 3.f*60.f;
// Jump
const float gravity = -40.f;
const float player_jump_speed = 9.6f;
const float player_max_speed_down = -12.f;
const float jump_button_until_height = 1.5f;
// Wall jump
const float player_max_speed_down_while_sticking_to_wall = -4.f;
const float time_sticking_to_wall = 4.f/60.f;
const float wall_jump_speed_x = 10.f;
const float after_wall_jump_acceleration = 0.2f*60.f;

const float falling_things_speed = -0.5f;
const Vec2 player_size = Vec2(0.15625f, 0.390625f);
const Vec2 player_box_size = Vec2(0.15625f, 0.1953125f);
const float player_y_offset = 0.f;

const Vec2 enemies_box_size = Vec2(0.2f, 0.2f);

extern f32 TIME_STEP;

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


void init_messages(GameState *game_state){
    for(int i=0; i<MAX_UPS; i++){
        game_state->particle_messages[i].time = INFINITY;
        game_state->sound_messages[i].time = INFINITY;
        
		game_state->player_snapshots[i].time = INFINITY;
        game_state->completion_snapshots[i] = false;
    }
}
void add_particle_message(GameState *game_state, Vec2 r, bool is_death){
    int next_free_slot = game_state->next_free_particle_slot;
    game_state->particle_messages[next_free_slot].time = game_state->game_time;
    game_state->particle_messages[next_free_slot].r = r;
    game_state->particle_messages[next_free_slot].is_death = is_death;
    next_free_slot = (next_free_slot + 1) % MAX_UPS;
    game_state->particle_messages[next_free_slot].time = INFINITY;
    game_state->next_free_particle_slot = next_free_slot;
}
void add_sound_message(GameState *game_state, u8 sound){
    int next_free_slot = game_state->next_free_sound_slot;
    game_state->sound_messages[next_free_slot].time = game_state->game_time;
    game_state->sound_messages[next_free_slot].sound = sound;
    next_free_slot = (next_free_slot + 1) % MAX_UPS;
    game_state->sound_messages[next_free_slot].time = INFINITY;
    game_state->next_free_sound_slot = next_free_slot;
}

// Functions for collision detection with tiles
Vec2 round_sorta(Vec2 v){
    const float factor = 1024.f;
    return Vec2(roundf(v.x*factor)/factor, roundf(v.y*factor)/factor);
}
bool collision_with_right_tile(Level *level, Vec2 old_r, Vec2 new_r, float *wall_x){
    Vec2 old_tr = round_sorta(old_r + player_size);
    Vec2 new_tr = round_sorta(new_r + player_size);
    
    int max_x = (int)new_tr.x;
    int min_x = (int)(old_tr.x) + 1;
    if(min_x > max_x) min_x = max_x;
    int dist = max_x - min_x;
    if(dist == 0) dist = 1;
    
    for(int x=min_x; x<=max_x; x++){
        float fy = lerp(old_tr.y, new_tr.y, (float)(x-min_x)/dist);
        int max_y = (int)sfloor(fy);
        int min_y = (int)(fy-2.f*player_size.y);
        
        for(int y=min_y; y<=max_y; y++){
            if(block_info[level->layout[x][y]] & BLOCK_IS_SOLID){
                *wall_x = (float)x - player_size.x;
                return true;
            }
        }
    }
    
    return false;
}
bool collision_with_left_tile(Level *level, Vec2 old_r, Vec2 new_r, float *wall_x){
    Vec2 old_bl = round_sorta(old_r - player_size);
    Vec2 new_bl = round_sorta(new_r - player_size);
    
    int min_x = (int)new_bl.x;
    int max_x = (int)old_bl.x - 1;
    if(max_x < min_x) max_x = min_x;
    int dist = max_x - min_x;
    if(dist == 0) dist = 1;
    
    for(int x=max_x; x>=min_x; x--){
        float fy = lerp(old_bl.y, new_bl.y, (float)(max_x-x)/dist);
        int min_y = (int)fy;
        int max_y = (int)sfloor(fy+2.f*player_size.y);
        
        for(int y=min_y; y<=max_y; y++){
            if(block_info[level->layout[x][y]] & BLOCK_IS_SOLID){
                *wall_x = (float)x + player_size.x + 1.f;
                return true;
            }
        }
    }
    
    return false;
}
bool collision_with_bottom_tile(Level *level, Vec2 old_r, Vec2 new_r, float *wall_y){
    Vec2 old_bl = round_sorta(old_r - player_size);
    Vec2 new_bl = round_sorta(new_r - player_size);
    
    int min_y = (int)new_bl.y;
    int max_y = (int)old_bl.y - 1;
    if(max_y < min_y) max_y = min_y;
    int dist = max_y - min_y;
    if(dist == 0) dist = 1;
    
    for(int y=max_y; y>=min_y; y--){
        float fx = lerp(old_bl.x, new_bl.x, (float)(max_y-y)/dist);
        int min_x = (int)fx;
        int max_x = (int)sfloor(fx+2.f*player_size.x);
        
        for(int x=min_x; x<=max_x; x++){
            if(block_info[level->layout[x][y]] & BLOCK_IS_SOLID){
                *wall_y = (float)y + player_size.y + 1.f;
                return true;
            }
        }
    }
    
    return false;
}
bool collision_with_top_tile(Level *level, Vec2 old_r, Vec2 new_r, float *wall_y){
    Vec2 old_tr = round_sorta(old_r + player_size);
    Vec2 new_tr = round_sorta(new_r + player_size);
    
    int max_y = (int)new_tr.y;
    int min_y = (int)old_tr.y + 1;
    if(min_y > max_y) min_y = max_y;
    int dist = max_y - min_y;
    if(dist == 0)
        dist = 1;
    
    for(int y=min_y; y<=max_y; y++){
        float fx = lerp(old_tr.x, new_tr.y, (float)(y-min_y)/dist);
        int max_x = (int)sfloor(fx);
        int min_x = (int)(fx-2.f*player_size.x);
        
        for(int x=min_x; x<=max_x; x++){
            if(block_info[level->layout[x][y]] & BLOCK_IS_SOLID){
                *wall_y = (float)y - player_size.y;
                return true;
            }
        }
    }
    
    return false;
}

void process_movement(GameState *game_state, u8 keys){
    Player *player = &game_state->player;
    Level  *level  = &game_state->level;
    auto enemies = level->enemies;
    auto platforms = level->platforms;
    
    if(player->flags & PM_DISABLED)
        keys = 0;
    
    bool moving = false;
    if(keys & frame_key_left){
        if(player->flags & PM_WALL_JUMPING_LEFT){
            player->v.x -= after_wall_jump_acceleration*TIME_STEP;
        }else if(player->flags & PM_STICKING_TO_WALL_RIGHT){
            player->time_to_unstick -= TIME_STEP;
            if(player->time_to_unstick < 0)
                player->flags &= ~PM_STICKING_TO_WALL;
            else
                player->v.x += 1.f;
        }else if(player->flags & PM_STICKING_TO_WALL_LEFT){
            player->v.x -= 1.f;
        }else if(player->flags & PM_ON_GROUND){
            if(player->v.x > 0.f) player->v.x = 0.f;
            player->v.x -= player_walk_acceleration*TIME_STEP;
        }else{
            player->v.x -= player_air_acceleration*TIME_STEP;
        }
        if(player->v.x < -player_walk_speed)
            player->v.x = -player_walk_speed;
        moving = true;
    }else if(player->flags & PM_STICKING_TO_WALL_RIGHT){
        player->time_to_unstick = time_sticking_to_wall;
        player->v.x += 1.f;
    }
    
    if(keys & frame_key_right){
        if(player->flags & PM_WALL_JUMPING_RIGHT){
            player->v.x += after_wall_jump_acceleration*TIME_STEP;
        }else if(player->flags & PM_STICKING_TO_WALL_LEFT){
            player->time_to_unstick -= TIME_STEP;
            if(player->time_to_unstick < 0)
                player->flags &= ~PM_STICKING_TO_WALL;
            else
                player->v.x -= 1.f;
        }else if(player->flags & PM_STICKING_TO_WALL_RIGHT){
            player->v.x += 1.f;
        }else if(player->flags & PM_ON_GROUND){
            if(player->v.x < 0.f) player->v.x = 0.f;
            player->v.x += player_walk_acceleration*TIME_STEP;
        }else{
            player->v.x += player_air_acceleration*TIME_STEP;
        }
        if(player->v.x > player_walk_speed)
            player->v.x = player_walk_speed;
        moving = true;
    }else if(player->flags & PM_STICKING_TO_WALL_LEFT){
        player->time_to_unstick = time_sticking_to_wall;
        player->v.x -= 1.f;
    }
    
    if(!moving){
        if(player->flags & PM_ON_GROUND)
            player->v.x *= 0.5f;
        else if(player->flags & PM_WALL_JUMPING)
            player->v.x *= 0.96f;
        else if(!(player->flags & PM_STICKING_TO_WALL))
            player->v.x *= 0.6f;
    }
    
    //static bool space_was_up = false;
    //static float last_frames_gravity = 0.f;
    
    if(player->jump_height >= 0.f && player->jump_height < jump_button_until_height){
        if(keys & frame_key_jump){
            player->v.y -= gravity*TIME_STEP; // TODO: Do this properly
            player->jump_height += player->v.y*TIME_STEP;
        }else
            player->jump_height = -1.f;
    }
    
    if(keys & frame_key_jump){
        bool jumped = false;
        if(player->flags & PM_ON_GROUND){
            jumped = true;
            player->v.y = player_jump_speed;
        }else if(player->space_was_up){
            if(player->flags & PM_STICKING_TO_WALL_RIGHT){
                jumped = true;
                player->v.x = -wall_jump_speed_x;
                player->v.y = player_jump_speed;
                player->flags |= PM_WALL_JUMPING_RIGHT;
            }else if(player->flags & PM_STICKING_TO_WALL_LEFT){
                jumped = true;
                player->v.x = wall_jump_speed_x;
                player->v.y = player_jump_speed;
                player->flags |= PM_WALL_JUMPING_LEFT;
            }
        }
        if(jumped){
            if(player->at_platform >= 0){
                player->r += platforms[player->at_platform].r;
                player->v += platforms[player->at_platform].v;
            }
            player->at_platform = -1;
            //player->on_ground = false;
            player->jump_height = 0.f;
            player->space_was_up = false;
            player->flags &= ~PM_STICKING_TO_WALL;
            add_sound_message(game_state, SOUND_JUMP);
        }
    }else{
        player->space_was_up = true;
    }
    
    //printf("(1) %i %i %i\n", player.at_platform, player.sticking_to_wall, player.on_ground);
    //printf("(2) %g %g; %g %g\n", player.r.x, player.r.y, player.v.x, player.v.y);
    
    bool kill = false;
    // Do the main physics update
    {
        Vec2 old_r = player->r;
        Vec2 v = player->v;
        
        Vec2 r = old_r + TIME_STEP*v;
        
        float wall_x;
        if(v.x < 0.f && collision_with_left_tile(level, old_r, r, &wall_x)){
            r.x = wall_x;
            v.x = 0.f;
            player->flags |= PM_PUSHES_TILE_LEFT;
        }else
            player->flags &= ~PM_PUSHES_TILE_LEFT;
        if(v.x > 0.f && collision_with_right_tile(level, old_r, r, &wall_x)){
            if(old_r.x <= wall_x){
                r.x = wall_x;
                player->flags |= PM_PUSHES_TILE_RIGHT;
            }
            v.x = 0.f;
        }else
            player->flags &= ~PM_PUSHES_TILE_RIGHT;
        
        float wall_y;
        if(v.y < 0.f && collision_with_bottom_tile(level, old_r, r, &wall_y)){
            r.y = wall_y;
            v.y = 0.f;
            player->flags |= PM_PUSHES_TILE_BOTTOM;
        }else
            player->flags &= ~PM_PUSHES_TILE_BOTTOM;
        if(v.y > 0.f && collision_with_top_tile(level, old_r, r, &wall_y)){
            r.y = wall_y;
            v.y = 0.f;
            player->jump_height = -1.f;
            player->flags |= PM_PUSHES_TILE_TOP;
        }else
            player->flags &= ~PM_PUSHES_TILE_TOP;
        
        // Detect collision with platforms
        //player->flags = (player->flags & 0xffffff00) | ((player->flags & 0x0f) << 4);
        for(uint i=0; i<platforms.size; i++){
            Vec2 plat_r = platforms[i].r;
            Vec2 size = platforms[i].size;
            
            bool separate = false;
            Vec2 overlap = r - plat_r;
            Vec2 dist = (player_size.x + size.x);
            if(overlap.x > 0.f){
                overlap.x = dist.x - overlap.x;
                if(overlap.x < 0.f)
                    separate = true;
            }else{
                overlap.x = -dist.x - overlap.x;
                if(overlap.x > 0.f)
                    separate = true;
            }
            if(overlap.y > 0.f){
                overlap.y = dist.y - overlap.y;
                if(overlap.y < 0.f)
                    separate = true;
            }else{
                overlap.y = -dist.y - overlap.y;
                if(overlap.y > 0.f)
                    separate = true;
            }
            if(separate)
                continue;
            
            //printf("%g %g\n", overlap.x, overlap.y);
        }
        
        
        player->r = r;
        player->v = v;
    }
    
    player->flags &= ~PM_STICKING_TO_WALL;
    if(!(player->flags & PM_ON_GROUND)){
        if(player->flags & PM_PUSHES_LEFT)
            player->flags |= PM_STICKING_TO_WALL_LEFT;
        if(player->flags & PM_PUSHES_RIGHT)
            player->flags |= PM_STICKING_TO_WALL_RIGHT;
    }
    Vec2 pr = player->r;
    
    //printf("(3) %g %g; %g %g\n", player.r.x, player.r.y, player.v.x, player.v.y);
    /*if(player.on_ground)
        player.sticking_to_wall = 0;
    if(player.sticking_to_wall == 1 && player.v.x < 0.f && player.at_platform < 0)
        player.sticking_to_wall = 0;
    if(player.sticking_to_wall == 2 && player.v.x > 0.f && player.at_platform < 0)
        player.sticking_to_wall = 0;*/
    
    for(int k=0; k<(int)platforms.size; k++){
        Vec2 nv = normalize(platforms[k].v);
        int x = (int)floorf(platforms[k].r.x + nv.x*platforms[k].size.x);
        int y = (int)floorf(platforms[k].r.y + nv.y*platforms[k].size.y);
        if(block_info[level->layout[x][y]] & BLOCK_STOPS_PLATFORMS){
            platforms[k].v = -platforms[k].v;
        }
    }
    
    if((player->flags & PM_STICKING_TO_WALL) && player->at_platform >= 0){
        if(abs(player->r.y) > player_size.y+platforms[player->at_platform].size.y){
            player->r += platforms[player->at_platform].r;
            player->v += platforms[player->at_platform].v;
            player->at_platform = -1;
        }
    }
    
    /*if(player.on_ground && player.at_platform >= 0){
        if(abs(player.r.x) <= player_size.x+platforms[player.at_platform].size.x)
            last_frames_gravity = 0.f;
        else{
            player.r += platforms[player.at_platform].r;
            player.v += platforms[player.at_platform].v;
            player.at_platform = -1;
        }
    }*/
    player->v.y += gravity*TIME_STEP;
    
    if(player->flags & PM_STICKING_TO_WALL){
        if(player->v.y < player_max_speed_down_while_sticking_to_wall)
            player->v.y = player_max_speed_down_while_sticking_to_wall;
    }else{
        if(player->v.y < player_max_speed_down)
            player->v.y = player_max_speed_down;
    }
    if(player->v.y <= 0.f){
        player->flags &= ~PM_WALL_JUMPING;
    }
    
    for(uint k=0; k<enemies.size; k++){
        enemies[k] += TIME_STEP*enemies_speed;
        if(enemies[k].x < 0.5f) enemies[k].x += level_size.x-1.f;
    }
    
    // Check if the player dies or wins
    if(!kill){
        float f_low_x = pr.x-player_box_size.x;
        int low_x = (int)floor(f_low_x);
        float f_high_x = pr.x+player_box_size.x;
        int high_x = (int)sfloor(f_high_x);
        float f_low_y = pr.y-player_box_size.y;
        int low_y = (int)floor(f_low_y);
        float f_high_y = pr.y+player_box_size.y;
        int high_y = (int)sfloor(f_high_y);
        for(int x=low_x; x<=high_x; x++){
            for(int y=low_y; y<=high_y; y++){
                char block = level->layout[x][y];
                u16 flags = block_info[block];
                if(flags & BLOCK_IS_GOAL){
                    if(!level->completed){
                        add_particle_message(game_state, Vec2((float)x+0.5f, (float)y+0.5f), false);
                        level->completed = true;
                        add_sound_message(game_state, SOUND_WIN);
                        if(block == '%')
                            game_mode = GAME_MODE_START;
                    }
                }else if(flags & BLOCK_IS_HARMFUL){
                    if(flags & BLOCK_IS_HARMFUL_UP){
                        if(f_low_y < y + 0.5f){
                            kill = true;
                            goto out_of_for;
                        }
                    }
                    if(flags & BLOCK_IS_HARMFUL_DOWN){
                        if(f_high_y > y + 0.5f){
                            kill = true;
                            goto out_of_for;
                        }
                    }
                    if(flags & BLOCK_IS_HARMFUL_RIGHT){
                        if(f_low_x < x + 0.5f){
                            kill = true;
                            goto out_of_for;
                        }
                    }
                    if(flags & BLOCK_IS_HARMFUL_LEFT){
                        if(f_high_x > x + 0.5f){
                            kill = true;
                            goto out_of_for;
                        }
                    }
                    if(flags & BLOCK_IS_HARMFUL_CENTER){
                        if(f_low_y < y + 0.85f || f_high_y > y + 0.15f || f_low_x < x + 0.85f || f_high_x > x + 0.15f){
                            kill = true;
                            goto out_of_for;
                        }
                    }
                }
            }
        }
        
        for(uint k=0; k<enemies.size; k++){
            Vec2 p = pr - enemies[k];
            const Vec2 m = player_box_size + enemies_box_size;
            if(-m <= p && p <= m){
                kill = true;
                goto out_of_for;
            }
        }
    }
    out_of_for:
    if(kill){
        add_particle_message(game_state, player->r, true);
        add_sound_message(game_state, SOUND_DEATH);
        
        game_state->player_lives--;
        if(!level->completed || level->time_to_load_next_level > time_after_flag_is_down){
            reset_player(&game_state->level_infos[level->num], player, level);
            level->completed = false;
        }else{
            player->r = Vec2(0.f);
            player->v = Vec2(0.f);
            player->flags |= PM_DISABLED;
        }
        
        should_save_game = true;
    }
    
    // Update level
    level->time += TIME_STEP;
    if(level->time >= 1.f){
        level->time -= 1.f;
        change_level_state(level);
        level->state = 1-level->state;
        game_state->draw_new_state_time = game_state->game_time + visual_lag_time;
    }
    
    if(level->completed){
        level->time_to_load_next_level -= TIME_STEP;
        if(level->time_to_load_next_level < 0.f){
            if(level->state == 1)
                game_state->draw_new_state_time = game_state->game_time + visual_lag_time;
            load_level(game_state, level->num+1);
            game_state->draw_new_level_time = game_state->game_time + visual_lag_time;
        }
    }
    
    // Record all the information for later rendering
    game_state->player_snapshots[game_state->next_free_snapshot] = {player->r, player->v, game_state->time, player->flags};
    if(player->at_platform >= 0){
        game_state->player_snapshots[game_state->next_free_snapshot].r += platforms[player->at_platform].r;
    }
    game_state->enemies_snapshots[game_state->next_free_snapshot].size = enemies.size;
    for(uint k=0; k<enemies.size; k++)
        game_state->enemies_snapshots[game_state->next_free_snapshot][k] = enemies[k];
    game_state->platforms_snapshots[game_state->next_free_snapshot].size = platforms.size;
    for(uint k=0; k<platforms.size; k++)
        game_state->platforms_snapshots[game_state->next_free_snapshot][k] = platforms[k].r;
    game_state->completion_snapshots[game_state->next_free_snapshot] = level->completed;
    game_state->next_free_snapshot = (game_state->next_free_snapshot+1) % MAX_UPS;
    
    process_messages(game_state);
}

void change_level_state(Level *level){
    for(int x=0; x<level->width; x++){
        for(int y=0; y<level->height; y++){
            if(level->layout[x][y] == 'a')
                level->layout[x][y] = 'b';
            else if(level->layout[x][y] == 'b')
                level->layout[x][y] = 'a';
        }
    }
}

void reset_player(LevelInfo *level_info, Player *player, Level *level){
    player->r = level->start_r;
    player->v = Vec2(0.f);
	player->flags = 0;
    
    player->jump_height = -1.f;
    player->time_to_unstick = -1.f;
    player->at_platform = -1;
    
    level->time = 0.f;
    if(level->state) change_level_state(level);
    level->state = 0;
    level->completed = false;
    level->time_to_load_next_level = level->time_to_load_next_level_max;
    //player.cancel_next_level_in = -1.f;
    
    //level->enemies.size = level_enemies[level->num].size;
    for(uint i=0; i<level->enemies.size; i++)
        level->enemies[i] = level_info->enemies[i];
    for(uint i=0; i<level->platforms.size; i++)
        level->platforms[i].r = level_info->platforms[i]; // TODO: reset speed and all
}

void process_messages(GameState *game_state){
    float time = game_state->time;
    { // Sound
        while(game_state->sound_messages[game_state->last_read_sound_slot].time < time-sound_lag_time){
            play_sound(game_state->sound_messages[game_state->last_read_sound_slot].sound);
            game_state->last_read_sound_slot= (game_state->last_read_sound_slot+1) % MAX_UPS;
        }
    }
    
    { // Particles
        auto &particles = game_state->particles;
        while(game_state->particle_messages[game_state->last_read_particle_slot].time < time-particle_lag_time){
            ParticleMessage message = game_state->particle_messages[game_state->last_read_particle_slot];
            game_state->last_read_particle_slot= (game_state->last_read_particle_slot+1) % MAX_UPS;
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

void load_player_into_buffer(GameState *game_state, BufferAndCount *buffer){
    Vertex_PT o_vertices[6];
    Vertex_PT *vertices = o_vertices;
    int next_candidate = game_state->last_rendered_snapshot+1;
    while(game_state->player_snapshots[next_candidate].time < game_state->time - visual_lag_time){
        next_candidate = (next_candidate + 1) % MAX_UPS;
    }
    game_state->last_rendered_snapshot = (next_candidate + MAX_UPS - 1) % MAX_UPS;
    game_state->rendered_player = game_state->player_snapshots[game_state->last_rendered_snapshot];
    PlayerSnapshot drawn_player = game_state->rendered_player;
    
    //if(drawn_player.level_time < TIME_STEP){
    //should_update_level = true;
    //}
    Vec2 r = drawn_player.r+Vec2(0.f, player_y_offset);
    const float texture_size = 256.f;
    const float outer_picture_size = 32.f;
    const float inner_picture_size = 30.f;
    const float picture_margin = 1.f;
    static int current_frame_x = 0;
    static int current_frame_y = 0;
    static int time_in_current_frame = 0;
    time_in_current_frame++;
    
    bool flip = false;
    if(drawn_player.v.x < 0.f)
        flip = true;
    if(drawn_player.flags & PM_STICKING_TO_WALL){
        current_frame_y = 4;
        current_frame_x = 0;
        flip = (drawn_player.flags & PM_STICKING_TO_WALL_RIGHT) != 0;
    }else if(!(drawn_player.flags & PM_ON_GROUND)){
        if(fabs(drawn_player.v.x) > 0.3f)
            current_frame_y = 6;
        else{
            current_frame_y = 5;
        }
        float factor = gravity*TIME_STEP;
        if(drawn_player.v.y > 3.f+factor){
            current_frame_x = 0;
        }else if(drawn_player.v.y > 1.5f+factor){
            current_frame_x = 1;
        }else if(drawn_player.v.y > -1.5f+factor){
            current_frame_x = 2;
        }else if(drawn_player.v.y > -3.f+factor){
            current_frame_x = 3;
        }else{
            current_frame_x = 4;
        }
    }else if(fabs(drawn_player.v.x) > 0.1f){
        current_frame_y = 7;
        if(time_in_current_frame >= 5){
            time_in_current_frame = 0;
            current_frame_x = current_frame_x%6 + 1;
            if(current_frame_x == 1 || current_frame_x == 4){
                static u8 odd = 0;
                odd++;
                if(odd)
                    add_sound_message(game_state, SOUND_WALK_0);
                else
                    add_sound_message(game_state, SOUND_WALK_1);
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

void load_particles_into_buffer(GameState *game_state, BufferAndCount *buffer){
    auto &particles = game_state->particles;
    int vertex_count = 6*particles.size;
    if(vertex_count == 0){
        buffer->count = 0;
        return;
    }
    
    start_temp_alloc();
    Vertex_PCa *o_vertices = (Vertex_PCa *)temp_alloc(vertex_count*sizeof(Vertex_PCa));
    Vertex_PCa *vertices = o_vertices;
    
    for(uint i=0; i<particles.size; i++){
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
