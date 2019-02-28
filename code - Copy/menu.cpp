#include "basecode/os.hpp"
#include "basecode/fonts.hpp"

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

void select_menu_option(GameState *game_state){
    MenuInfo *menu = &game_state->menu_info;
    switch(menu->selected_option){
        case -1: break;
        case 0:
        if(!game_state->game_started_this_run) // TODO: We should continue/start new game automatically when launching the program (but still show the menu)...
            continue_game(game_state);
        c_close_menu();
        break;
        case 1:
        new_game(game_state, false);
        c_close_menu();
        break;
        case 2:
        new_game(game_state, true);
        c_close_menu();
        break;
    }
}

void process_menu(GameState *game_state,
                  #if !OS_IS_MOBILE
                  u8 keys
                  #else
                  Vec2 point
                  #endif
                  ){
#if !OS_IS_MOBILE
    MenuInfo *menu = &game_state->menu_info;
    const float key_wait_time = 0.1f;
    if(keys & frame_key_up){
        if(menu->holding_key_times[0] > 0.f) menu->holding_key_times[0] -= TIME_STEP;
        else{
            do{
                menu->selected_option = (menu->selected_option+menu_total_options-1) % menu_total_options;
            }while(menu->disabled_options[menu->selected_option]);
            menu->holding_key_times[0] = key_wait_time;
        }
    }else menu->holding_key_times[0] = -1.f;
    
    if(keys & frame_key_down){
        if(menu->holding_key_times[1] > 0.f) menu->holding_key_times[1] -= TIME_STEP;
        else{
            do{
                menu->selected_option = (menu->selected_option+1) % menu_total_options;
            }while(menu->disabled_options[menu->selected_option]);
            menu->holding_key_times[1] = key_wait_time;
        }
    }else menu->holding_key_times[1] = -1.f;
    
    if(keys & frame_key_space){
        if(menu->holding_key_times[4] > 0.f) menu->holding_key_times[4] -= TIME_STEP;
        else{
            select_menu_option(game_state);
        }
    }else menu->holding_key_times[4] = -1.f;
#else
    int mso = menu->selected_option;
    menu->selected_option = -1;
    point.x -= (1.f/actual_screen_ratio-1.f)*0.5f + 0.47f;
    int candidate = (int)((point.y-1.f+0.83f)*10.f);
    int j = -1;
    for(int i=0; i<menu_total_options; i++){
        if(menu->disabled_options[i])
            continue;
        j++;
        if(j == candidate){
            if(point.x >= 0.f && point.x <= menu->options_width[i])
                menu->selected_option = i;
            break;
        }
    }
#if OS == OS_IOS
    if(mso != menu->selected_option){
        void ios_do_small_haptic_feedback();
        ios_do_small_haptic_feedback();
    }
#endif
#endif
    
    menu->disabled_options[0] = !game_state->game_started;
    menu->disabled_options[1] = false;
    menu->disabled_options[2] = true;
    menu->disabled_options[3] = false;
    menu->disabled_options[4] = false;
#if OS_IS_DESKTOP
    menu->disabled_options[5] = false;
#endif
}
void draw_menu(GameState *game_state){
    // Update buffers
    GLObjects *gl = &game_state->gl_objects;
    MenuInfo *menu = &game_state->menu_info;
    
    start_temp_alloc();
    u32 vert_num = 0;
    
    String title_string;
    title_string.allocator = &temporary_storage.allocator;
    title_string = "game\nof flag\n";
    vert_num += text_vert_num(title_string);
    
    String options_strings[menu_total_options];
    
    if(!menu->disabled_options[0]){
        options_strings[0].allocator = &temporary_storage.allocator;
        options_strings[0] = "continue";
        vert_num += text_vert_num(options_strings[0]);
    }
    
    options_strings[1].allocator = &temporary_storage.allocator;
    options_strings[1] = "new game";
    vert_num += text_vert_num(options_strings[1]);
    
    options_strings[2].allocator = &temporary_storage.allocator;
    options_strings[2] = "newer game";
    vert_num += text_vert_num(options_strings[2]);
    
    options_strings[3].allocator = &temporary_storage.allocator;
    options_strings[3] = "level select";
    vert_num += text_vert_num(options_strings[3]);
    
    options_strings[4].allocator = &temporary_storage.allocator;
    options_strings[4] = "options";
    vert_num += text_vert_num(options_strings[4]);
    
#if OS_IS_DESKTOP
    options_strings[5].allocator = &temporary_storage.allocator;
    options_strings[5] = "quit";
    vert_num += text_vert_num(options_strings[5]);
#endif
    
    
    String time_string;
    time_string.allocator = &temporary_storage.allocator;
    char time_sc_0[30], time_sc_1[30];
    {
        float time = game_state->time;
        if(time < 60.)
            sprintf(time_sc_0, "%.2lf ", time);
        else if(time < 3600.){
            int sec = (int)time;
            int min = sec/60;
            sprintf(time_sc_0, "%i:%05.2lf ", min, time-min*60.);
        }else{
            int sec = (int)time;
            int min = sec/60;
            int h   = min/60;
            sprintf(time_sc_0, "%i:%02i:%05.2lf ", h, min%60, time-min*60.);
        }
    } {
        float time = game_state->stats.best_time;
        if(time < 60.)
            sprintf(time_sc_1, "%.2lf ", time);
        else if(time < 3600.){
            int sec = (int)time;
            int min = sec/60;
            sprintf(time_sc_1, "%i:%05.2lf ", min, time-min*60.);
        }else{
            int sec = (int)time;
            int min = sec/60;
            int h   = min/60;
            sprintf(time_sc_1, "%i:%02i:%05.2lf ", h, min%60, time-min*60.);
        }
    }
    char time_chars[100];
    sprintf(time_chars, "%s\n%s\n", time_sc_0, time_sc_1);
    time_string = time_chars;
    vert_num += text_vert_num(time_string);
    
    String time_text_string;
    time_text_string.allocator = &temporary_storage.allocator;
    sprintf(time_chars, "Time:\nBest:\n");
    time_text_string = time_chars;
    vert_num += text_vert_num(time_text_string);
    
    Vertex_PTCa *o_verts = (Vertex_PTCa *)temp_alloc(vert_num*sizeof(Vertex_PTCa));
    Vertex_PTCa *verts = o_verts;
    
    verts += render_text(0.42f, 0.9f, -0.9f, FONT_QUALITY_64, title_string, verts, 0.13f, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
    float y = 0.855f;
    for(int i=0; i<menu_total_options; i++){
        if(menu->disabled_options[i])
            continue;
        verts += render_text(0.47f, y, -0.9f, FONT_QUALITY_64, options_strings[i], verts, 0.07f, &menu->options_width[i], NULL, i == menu->selected_option ? RgbaColor{255, 255, 255, 255} : RgbaColor{155, 155, 155, 255}, TEXT_ALIGN_TOP);
        y -= 0.1f;
    }
    verts += render_text_monospace(0.42f, 0.6f, -0.9f, FONT_QUALITY_64, time_string, verts, 0.05f, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
    verts += render_text(0.18f, 0.6f, -0.9f, FONT_QUALITY_64, time_text_string, verts, 0.05f, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
    
    gl->text_buffer.count = vert_num;
    set_buffer_data_static(gl->text_buffer.buffer, o_verts, gl->text_buffer.count);
    
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
        check_openGL_error();
    }
}
