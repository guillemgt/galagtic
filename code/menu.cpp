#include "basecode/os.hpp"
#include "basecode/fonts.hpp"

#include "menu.hpp"
#include "world.hpp"
#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

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

int menu_selected_option;
bool disabled_options[menu_total_options];
float options_width[menu_total_options];

float holding_key_times[8] = {-1.f, -1.f, -1.f, -1.f, -1.f};

void select_menu_option(){
    switch(menu_selected_option){
        case -1: break;
        case 0:
            if(!game_started_this_run)
                continue_game();
            c_close_menu();
            break;
        case 1:
            new_game(false);
            c_close_menu();
            break;
        case 2:
            new_game(true);
            c_close_menu();
            break;
    }
}

void process_menu(
#if !OS_IS_MOBILE
                  u8 keys
#else
                  Vec2 point
#endif
){
#if !OS_IS_MOBILE
    const float key_wait_time = 0.1f;
    if(keys & frame_key_up){
        if(holding_key_times[0] > 0.f) holding_key_times[0] -= TIME_STEP;
        else{
            do{
                menu_selected_option = (menu_selected_option+menu_total_options-1) % menu_total_options;
            }while(disabled_options[menu_selected_option]);
            holding_key_times[0] = key_wait_time;
        }
    }else holding_key_times[0] = -1.f;
    
    if(keys & frame_key_down){
        if(holding_key_times[1] > 0.f) holding_key_times[1] -= TIME_STEP;
        else{
            do{
                menu_selected_option = (menu_selected_option+1) % menu_total_options;
            }while(disabled_options[menu_selected_option]);
            holding_key_times[1] = key_wait_time;
        }
    }else holding_key_times[1] = -1.f;
    
    if(keys & frame_key_space){
        if(holding_key_times[4] > 0.f) holding_key_times[4] -= TIME_STEP;
        else{
            select_menu_option();
        }
    }else holding_key_times[4] = -1.f;
#else
    int mso = menu_selected_option;
    menu_selected_option = -1;
    point.x -= (1.f/actual_screen_ratio-1.f)*0.5f + 0.47f;
    int candidate = (int)((point.y-1.f+0.83f)*10.f);
    int j = -1;
    for(int i=0; i<menu_total_options; i++){
        if(disabled_options[i])
            continue;
        j++;
        if(j == candidate){
            if(point.x >= 0.f && point.x <= options_width[i])
                menu_selected_option = i;
            break;
        }
    }
#if OS == OS_IOS
    if(mso != menu_selected_option){
        void ios_do_small_haptic_feedback();
        ios_do_small_haptic_feedback();
    }
#endif
#endif
    
    disabled_options[0] = !game_started;
    disabled_options[1] = false;
    disabled_options[2] = true;
    disabled_options[3] = false;
    disabled_options[4] = false;
#if OS_IS_DESKTOP
    disabled_options[5] = false;
#endif
}
void draw_menu(){
    // Update buffers
    
    start_temp_alloc();
    u32 vert_num = 0;
    
    String title_string;
    title_string.allocator = &temporary_storage.allocator;
    title_string = "a laggy\ngame\n";
    vert_num += text_vert_num(title_string);
    
    String options_strings[menu_total_options];
    
    if(!disabled_options[0]){
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
        float time = player.shown_time;
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
        float time = player.best_time;
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
        if(disabled_options[i])
            continue;
        verts += render_text(0.47f, y, -0.9f, FONT_QUALITY_64, options_strings[i], verts, 0.07f, &options_width[i], NULL, i == menu_selected_option ? RgbaColor{255, 255, 255, 255} : RgbaColor{155, 155, 155, 255}, TEXT_ALIGN_TOP);
        y -= 0.1f;
    }
    verts += render_text_monospace(0.42f, 0.6f, -0.9f, FONT_QUALITY_64, time_string, verts, 0.05f, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
    verts += render_text(0.18f, 0.6f, -0.9f, FONT_QUALITY_64, time_text_string, verts, 0.05f, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP|TEXT_ALIGN_RIGHT);
    
    text_buffer.count = vert_num;
    set_buffer_data_static(text_buffer.buffer, o_verts, text_buffer.count);
    
    end_temp_alloc();
    
    // Start drawing
    const float prop = 1.f/1.2f;
    Vec2 v = (actual_screen_ratio < prop ? Vec2(2.f*actual_screen_ratio, 2.f) : Vec2(2.f, 2.f/actual_screen_ratio));
    Mat4 matrix = get_translation_matrix(Vec3(-0.5f*v.x, -0.5f*v.y, 0.f)) * get_scale_matrix(v.x, v.y, 1.f);
    
    glClearColor(basic_color.r/255.f, basic_color.g/255.f, basic_color.b/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(text_buffer.count > 0){
        glUseProgram(ptca_program.id);
        glUniformMatrix4fv(ptca_program.matrix, 1, GL_FALSE, &matrix.values[0][0]);
        glUniform1i(ptca_program.sampler, 2);
        
        glBindVertexArray(text_vao);
        glDrawArrays(GL_TRIANGLES, 0, text_buffer.count);
        check_openGL_error();
    }
}
