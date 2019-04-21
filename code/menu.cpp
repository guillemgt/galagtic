#include "include/misc_tools.hpp"

#include "basecode.hpp"

/*
 ================================
 |                              |
 |     a laggy  Continue        |
 |        game  New game        |
 |              Level select    |
 |   o          Options         |
 |   +          Quit            |
 |   ^                          |
 ================================
 */

void menu_load_level(GameState *game_state){
    MenuInfo *menu = &game_state->menu_info;
    
    load_level(game_state, menu->shown_level);
    init_messages(game_state);
    draw_scene(game_state, true, false);
    
    game_state->time = 0.f;
}

const float menu_right_top_y = 0.855f;
const float menu_option_height = 0.1f;

const float level_select_miniscreen_top = 0.9f;
const float level_select_miniscreen_left = 0.1f;
const float level_select_miniscreen_bottom = 0.4f;

int select_menu_option(GameState *game_state){
    MenuInfo *menu = &game_state->menu_info;
    
    if(menu->selected_option < 0)
        return 1;
    
    switch(menu->screen){
        case MS_MAIN:
        switch(menu->selected_option){
            case MB_MAIN_CONTINUE:
            case MB_MAIN_NEW_GAME:
            case MB_MAIN_NEW_GAME_PLUS:
            menu->fade_alpha = 0.f;
            menu->fade_direction = 1.f;
            break;
            
            case MB_MAIN_LEVEL_SELECT:
            menu->last_selected_option = menu->selected_option;
            menu->selected_option = 0;
            menu->last_screen = menu->screen;
            menu->screen = MS_LEVEL_SELECT;
            game_state->input_lag_time = 0.f;
            game_state->render_lag_time = 0.f;
            menu->shown_level = 1;
            menu_load_level(game_state);
            break;
            
            case MB_MAIN_SETTINGS:
            menu->last_selected_option = menu->selected_option;
            menu->selected_option = 0;
            menu->last_screen = menu->screen;
            menu->screen = MS_SETTINGS;
            break;
            
            case MB_MAIN_QUIT:
            return 0;
            break;
        }
        break;
        
        case MS_INGAME:
        switch(menu->selected_option){
            case MB_INGAME_RESUME:
            close_menu(game_state);
            break;
            
            case MB_INGAME_SETTINGS:
            menu->last_selected_option = menu->selected_option;
            menu->selected_option = 0;
            menu->last_screen = menu->screen;
            menu->screen = MS_SETTINGS;
            break;
            
            case MB_INGAME_TO_MAIN:
            save_game(game_state);
            menu->fade_alpha = 0.f;
            menu->fade_direction = 1.f;
            break;
            
            case MB_INGAME_QUIT:
            save_game(game_state);
            return 0;
            break;
        }
        break;
        
        case MS_SETTINGS:
        switch(menu->selected_option){
            case MB_SETTINGS_SOUND:
            break;
            case MB_SETTINGS_BACK:
            menu->selected_option = menu->last_selected_option;
            menu->screen = menu->last_screen;
            break;
        }
        
        case MS_LEVEL_SELECT:
        switch(menu->selected_option){
            case MB_LS_PLAY:
            case MB_LS_PLAY_PLUS:
            menu->fade_alpha = 0.f;
            menu->fade_direction = 1.f;
            break;
            case MB_LS_BACK:
            menu->selected_option = menu->last_selected_option;
            menu->screen = menu->last_screen;
            break;
            case MB_LS_RIGHT:
            if(menu->shown_level < game_state->stats.unlocked_levels){
                menu->shown_level++;
                menu_load_level(game_state);
            }
            break;
            case MB_LS_LEFT:
            if(menu->shown_level > 1){
                menu->shown_level--;
                menu_load_level(game_state);
            }
            break;
        }
        default:
        break;
    }
    return 1;
}

int menu_keydown(GameState *game_state, char key){
    MenuInfo *menu = &game_state->menu_info;
    if(menu->fade_alpha >= 0.f)
        return 1;
    
    switch(key){
        case GGTP_KEY_UP:
        do{
            menu->selected_option = (menu->selected_option+menu_total_options[menu->screen]-1) % menu_total_options[menu->screen];
        }while(menu->screen == MS_MAIN && menu->disabled_options[menu->selected_option]);
        break;
        
        case GGTP_KEY_DOWN:
        do{
            menu->selected_option = (menu->selected_option+1) % menu_total_options[menu->screen];
        }while(menu->screen == MS_MAIN && menu->disabled_options[menu->selected_option]);
        break;
        
        case GGTP_KEY_RIGHT:
        if(menu->screen == MS_LEVEL_SELECT && menu->shown_level < game_state->stats.unlocked_levels){
            menu->shown_level++;
            menu_load_level(game_state);
        }
        break;
        
        case GGTP_KEY_LEFT:
        if(menu->screen == MS_LEVEL_SELECT && menu->shown_level > 1){
            menu->shown_level--;
            menu_load_level(game_state);
        }
        break;
        
        case GGTP_KEY_SPACE:
        case GGTP_KEY_RETURN:
        return select_menu_option(game_state);
        break;
        
        case GGTP_KEY_ESC:
        if(menu->screen == MS_INGAME)
            close_menu(game_state);
        break;
    }
    return 1;
}

int menu_mousemove(GameState *game_state, Vec2 position){
    MenuInfo *menu = &game_state->menu_info;
    if(menu->fade_alpha >= 0.f)
        return 0;
    
    menu->selected_option = -1;
    
    GLObjects *gl = &game_state->gl_objects;
    
    const float prop = 1.f/1.2f;
    Vec2 r;
    Vec2 v = gl->actual_screen_size;
    if(gl->actual_screen_ratio < prop){
        r = Vec2(position.x - 0.5f*v.x, v.y - position.y) / v.y;
        r.x += 0.5f;
    }else{
        r = Vec2(position.x, 0.5f*v.y - position.y) / v.x;
        r.y += 0.5f;
    }
    
    //printf("%g %g\n", r.x, r.y);
    
    float top_y;
    
    if(menu->screen == MS_LEVEL_SELECT){
        if(r.y > level_select_miniscreen_bottom){
            if(r.y > level_select_miniscreen_top)
                return 0;
            if(r.x > 0.3f && r.x < 0.7f)
                return 0;
            
            menu->selected_option = r.x > 0.5f ? 3 : 4;
            
            return 1;
        }else{
            top_y = level_select_miniscreen_bottom;
        }
    }else{
        top_y = menu_right_top_y;
    }
    
    if(r.x < 0.42f)
        return 0;
    
    float opt_candidate = (top_y - r.y) / menu_option_height;
    float frac = fmodf(opt_candidate, 1.f);
    if(frac < 0.15f || frac > 0.85f)
        return 0;
    int opt = (int)opt_candidate;
    if(opt < 0)
        return 0;
    
    menu->selected_option = 0;
    for(int i=0; i<opt; i++){
        do {
            menu->selected_option++;
        }while(menu->screen == MS_MAIN && menu->disabled_options[menu->selected_option]);
        if(menu->selected_option >= menu_total_options[menu->screen])
            return 0;
    }
    
    
    return 1;
}

void process_menu(GameState *game_state){
    MenuInfo *menu = &game_state->menu_info;
    
    if(menu->fade_alpha >= 0.f){
        menu->fade_alpha += menu->fade_direction*fade_speed*TIME_STEP;
        if(menu->fade_alpha > 1.f){
            if(menu->screen == MS_MAIN){
                switch(menu->selected_option){
                    case 0:
                    continue_game(game_state);
                    close_menu(game_state);
                    break;
                    case 1:
                    new_game(game_state, false);
                    close_menu(game_state);
                    break;
                    case 2:
                    new_game(game_state, true);
                    close_menu(game_state);
                    break;
                }
            }else if(menu->screen == MS_INGAME){
                switch(menu->selected_option){
                    case MB_INGAME_TO_MAIN:
                    menu->screen = MS_MAIN;
                    menu->selected_option = 0;
                    menu->fade_direction = -1.f;
                    break;
                }
            }else if(menu->screen == MS_LEVEL_SELECT){
                switch(menu->selected_option){
                    case MB_LS_PLAY:
                    new_level_select_game(game_state, menu->shown_level, false);
                    close_menu(game_state);
                    break;
                    case MB_LS_PLAY_PLUS:
                    new_level_select_game(game_state, menu->shown_level, true);
                    close_menu(game_state);
                    break;
                }
                return;
            }
        }
    }
    
    if(menu->screen == MS_LEVEL_SELECT){
        add_snapshot(game_state);
    }
    
    menu->disabled_options[0] = !game_state->current_run_data.game_started;
    menu->disabled_options[1] = false;
    menu->disabled_options[2] = game_state->stats.best_time == INFINITY;
    menu->disabled_options[3] = game_state->stats.unlocked_levels < 1;
    menu->disabled_options[4] = false;
#if OS_IS_DESKTOP
    menu->disabled_options[5] = false;
#endif
}

void write_time_and_lives(float time, int lives, char *string, char *title_string){
    if(time == INFINITY){
        string[0] = 0;
        title_string[0] = 0;
        return;
    }
    
    if(time < 0.)
        time = 0.f;
    
    char time_string[30];
    if(time < 60.)
        sprintf(time_string, "%.2lf", time);
    else if(time < 3600.){
        int sec = (int)time;
        int min = sec/60;
        sprintf(time_string, "%i:%05.2lf", min, time-min*60.);
    }else{
        int sec = (int)time;
        int min = sec/60;
        int h   = min/60;
        sprintf(time_string, "%i:%02i:%05.2lf", h, min%60, time-min*60.);
    }
    
    if(lives >= 0)
        sprintf(string, "%s\n x%i\n\n", time_string, lives);
    else
        sprintf(string, "%s\n x(%i)\n\n", time_string, lives);
}

void draw_menu(GameState *game_state){
    const RgbaColor text_color = {135, 135, 135, 255};
    const RgbaColor selected_text_color = {255, 255, 255, 255};
    const RgbaColor title_text_color = {255, 255, 255, 255};
    
    // Update buffers
    GLObjects *gl = &game_state->gl_objects;
    MenuInfo *menu = &game_state->menu_info;
    
    start_temp_alloc();
    u32 vert_num = 0;
    const u32 max_vert_num = 1000;
    
    Vertex_PTCa *o_verts = (Vertex_PTCa *)talloc(max_vert_num*sizeof(Vertex_PTCa));
    Vertex_PTCa *verts = o_verts;
    
    float left_y;
    if(menu->screen == MS_MAIN){
        verts += render_text(0.42f, 0.9f, -0.9f, FONT_QUALITY_64, "lagoon\nmoon\n", (mt_Vertex_PTCa *)verts, 0.13f, NULL, NULL, title_text_color, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
        left_y = 0.6f;
    }else{
        left_y = 0.855f;
    }
    float right_y = menu_right_top_y;
    
    switch(menu->screen){
        case MS_MAIN:
        case MS_INGAME: {
            CurrentRunData *crt = &game_state->current_run_data;
            
            char time_sc_0[40], time_sc_1[40], time_sc_2[40];
            char time_tt_0[10] = "\n\n\n", time_tt_1[10] = "best\n\n\n", time_tt_2[10] = "best+\n\n\n";
            write_time_and_lives(crt->time - crt->time_started_counting, crt->player_lives, time_sc_0, time_tt_0);
            write_time_and_lives(game_state->stats.best_time, game_state->stats.best_lives, time_sc_1, time_tt_1);
            write_time_and_lives(game_state->stats.best_time_plus, game_state->stats.best_lives_plus, time_sc_2, time_tt_2);
            
            if(!game_state->current_run_data.game_started){
                time_sc_0[0] = 0;
                time_tt_0[0] = 0;
            }
            
            char time_chars[100];
            sprintf(time_chars, "%s%s%s", time_sc_0, time_sc_1, time_sc_2);
            //vert_num += text_vert_num(time_string);
            
            char time_text_chars[100];
            sprintf(time_text_chars, "%s%s%s", time_tt_0, time_tt_1, time_tt_2);
            //vert_num += text_vert_num(time_text_string);
            
            left_y -= 0.02f;
            verts += render_text(0.42f, left_y, -0.9f, FONT_QUALITY_64, time_chars, (mt_Vertex_PTCa *)verts, 0.05f, NULL, NULL, title_text_color, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT|TEXT_MONOSPACE);
            verts += render_text(0.18f, left_y, -0.9f, FONT_QUALITY_64, time_text_chars, (mt_Vertex_PTCa *)verts, 0.05f, NULL, NULL, title_text_color, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
        } break;
        case MS_SETTINGS: {
            verts += render_text(0.42f, left_y, -0.9f, FONT_QUALITY_64, "settings\n", (mt_Vertex_PTCa *)verts, 0.07f, NULL, NULL, title_text_color, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
        } break;
        case MS_LEVEL_SELECT:
        right_y = level_select_miniscreen_bottom;
        break;
        case MS_SURE_NEW_GAME:
        break;
    }
    
    int selected_option = menu->selected_option;
    
#define ADD_MENU_TEXT(text) \
    { \
        verts += render_text(0.47f, right_y, -0.9f, FONT_QUALITY_64, text, (mt_Vertex_PTCa *)verts, 0.07f, NULL, NULL, text_color, TEXT_ALIGN_TOP); \
        right_y -= 0.06f; \
    }
#define ADD_MENU_OPTION(text) \
    { \
        verts += render_text(0.47f, right_y, -0.9f, FONT_QUALITY_64, text, (mt_Vertex_PTCa *)verts, 0.07f, NULL, NULL, option_num == selected_option ? selected_text_color : text_color, TEXT_ALIGN_TOP); \
        option_num++; \
        right_y -= menu_option_height; \
    }
#define ADD_MENU_OPTION_IF_ENABLED(num, text) \
    if(!menu->disabled_options[num]){ \
        ADD_MENU_OPTION(text); \
    }else if(selected_option > num) \
    selected_option--;
    
    int option_num = 0;
    switch(menu->screen){
        case MS_MAIN:
        
        ADD_MENU_OPTION_IF_ENABLED(0, "continue");
        ADD_MENU_OPTION("new game");
        ADD_MENU_OPTION_IF_ENABLED(2, "new game+");
        ADD_MENU_OPTION_IF_ENABLED(3, "level select");
        ADD_MENU_OPTION("settings");
#if OS_IS_DESKTOP
        ADD_MENU_OPTION("quit");
#endif
        break;
        
        case MS_INGAME:
        ADD_MENU_OPTION("resume");
        ADD_MENU_OPTION("settings");
        ADD_MENU_OPTION("back to menu");
        ADD_MENU_OPTION("quit");
        break;
        
        case MS_LEVEL_SELECT: {
            ADD_MENU_OPTION("play");
            ADD_MENU_OPTION("play+");
            ADD_MENU_OPTION("back");
            
            const float y = 0.5f*(level_select_miniscreen_top + level_select_miniscreen_bottom) - 0.035f;
            
            if(menu->shown_level < game_state->stats.unlocked_levels){
                verts += render_text(1.f-level_select_miniscreen_left, y, -0.9f, FONT_QUALITY_64, ">", (mt_Vertex_PTCa *)verts, 0.07f, NULL, NULL, option_num == selected_option ? selected_text_color : text_color, TEXT_ALIGN_LEFT);
            }
            option_num++;
            if(menu->shown_level > 1){
                verts += render_text(level_select_miniscreen_left-0.04f, y, -0.9f, FONT_QUALITY_64, "<", (mt_Vertex_PTCa *)verts, 0.07f, NULL, NULL, option_num == selected_option ? selected_text_color : text_color, TEXT_ALIGN_RIGHT);
            }
            option_num++;
        } break;
        
        
        case MS_SETTINGS:
        ADD_MENU_OPTION("sound: on");
        ADD_MENU_OPTION("back");
        break;
        
        case MS_SURE_NEW_GAME:
        ADD_MENU_TEXT("are you sure you");
        ADD_MENU_TEXT("want to start a");
        ADD_MENU_TEXT("new game?");
        ADD_MENU_OPTION("yes");
        ADD_MENU_OPTION("no");
        break;
    }
    
    gl->text_buffer.count = (u32)(verts-o_verts);
    ggtgl_set_buffer_data(gl->text_buffer.buffer, o_verts, gl->text_buffer.count, GL_DYNAMIC_DRAW);
    
    end_temp_alloc();
    
    // Start drawing
    const float prop = 1.f/1.2f;
    Vec2 v = (gl->actual_screen_ratio < prop ? Vec2(2.f*gl->actual_screen_ratio, 2.f) : Vec2(2.f, 2.f/gl->actual_screen_ratio));
    Mat4 matrix = get_translation_matrix(Vec3(-0.5f*v.x, -0.5f*v.y, 0.f)) * get_scale_matrix(v.x, v.y, 1.f);
    
    glClearColor(basic_color.r/255.f, basic_color.g/255.f, basic_color.b/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(gl->text_buffer.count > 0){
        glUseProgram(gl->ptca_program.id);
        glUniformMatrix4fv(gl->ptca_program.matrix, 1, GL_FALSE, &matrix.values[0][0]);
        glUniform1i(gl->ptca_program.sampler, 2);
        
        glBindVertexArray(gl->text_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->text_buffer.count);
        ggtgl_check_error();
    }
    
    if(game_state->menu_info.screen == MS_LEVEL_SELECT){
        int x_0;
        int y_0, y_1;
        Vec2 r = {level_select_miniscreen_left, 1.f-level_select_miniscreen_bottom};
        float r2y = 1.f-level_select_miniscreen_top;
        v = gl->actual_screen_size;
        if(gl->actual_screen_ratio < prop){
            x_0 = (int)(v.y*(r.x-0.5f) + 0.5f*v.x);
            y_0 = (int)(v.y - v.y*r.y);
            y_1 = (int)(v.y - v.y*r2y);
        }else{
            x_0 = (int)(v.x*r.x);
            y_0 = (int)(0.5f*v.y - v.x*(r.y-0.5f));
            y_1 = (int)(0.5f*v.y - v.x*(r2y-0.5f));
        }
        int size_x = window_size.x - 2*x_0;
        int size_y = y_1 - y_0;
        
        change_window_size(game_state, {(float)size_x, (float)size_y});
        glViewport(x_0, y_0, size_x, size_y);
        glScissor(x_0, y_0, size_x, size_y);
        draw_scene(game_state, false);
        change_window_size(game_state, (Vec2)window_size);
        glViewport(0, 0, window_size.x, window_size.y);
        glScissor(0, 0, window_size.x, window_size.y);
    }
    
    float alpha = game_state->menu_info.fade_alpha;
    if(alpha > 0.f){
        RgbaColor c = {0, 0, 0, (u8)(alpha*255)};
        
        Vertex_PCa o_vertices[] = {
            {Vec3(-1.f, -1.f, -1.f), c},
            {Vec3(+1.f, -1.f, -1.f), c},
            {Vec3(-1.f, +1.f, -1.f), c},
            {Vec3(-1.f, +1.f, -1.f), c},
            {Vec3(+1.f, -1.f, -1.f), c},
            {Vec3(+1.f, +1.f, -1.f), c},
        };
        
        gl->fade_buffer.count = ArraySize(o_vertices);
        ggtgl_set_buffer_data(gl->fade_buffer.buffer, o_vertices, gl->fade_buffer.count, GL_DYNAMIC_DRAW);
        
        Mat4 id = mat4_identity;
        
        glUseProgram(gl->pca_program.id);
        glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &id.values[0][0]);
        glBindVertexArray(gl->fade_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->fade_buffer.count);
        ggtgl_check_error();
    }
}
