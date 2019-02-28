#include "basecode/basecode.hpp"
#include "basecode/os.hpp"

#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

Vec2 level_size;

extern Vec2i window_size;


/*struct ModelInfo {
    char path[MAX_PATH_LENGTH];
    StaticModel *model;
};
ModelInfo models_info[10];
StaticModel static_models[10];*/

void init_openGL(GameState *game_state){
    GLObjects *gl = &game_state->gl_objects;
    // We read the wall shaders
    const char wall_shader[] =
#include "Shaders/pca_shader.glsl"
        ;
    gl->pca_program.id = load_shaders_by_text(wall_shader);
    gl->pca_program.position = glGetAttribLocation (gl->pca_program.id, "a_position");
    gl->pca_program.color    = glGetAttribLocation (gl->pca_program.id, "a_color");
    gl->pca_program.matrix   = glGetUniformLocation(gl->pca_program.id, "u_matrix");
    
    const char pt_shader[] =
#include "Shaders/pt_shader.glsl"
        ;
    gl->pt_program.id = load_shaders_by_text(pt_shader);
    gl->pt_program.position   = glGetAttribLocation (gl->pt_program.id, "a_position");
    gl->pt_program.tex_coords = glGetAttribLocation (gl->pt_program.id, "a_tex_coords");
    gl->pt_program.matrix     = glGetUniformLocation(gl->pt_program.id, "u_matrix");
    gl->pt_program.sampler    = glGetUniformLocation(gl->pt_program.id, "u_sampler");
    gl->pt_program.color_multiplier = glGetUniformLocation(gl->pt_program.id, "u_color_multiplier");
    
    const char ptt_shader[] =
#include "Shaders/ptt_shader.glsl"
        ;
    gl->ptt_program.id = load_shaders_by_text(ptt_shader);
    gl->ptt_program.position   = glGetAttribLocation (gl->ptt_program.id, "a_position");
    gl->ptt_program.tex_coords = glGetAttribLocation (gl->ptt_program.id, "a_tex_coords");
    gl->ptt_program.matrix     = glGetUniformLocation(gl->ptt_program.id, "u_matrix");
    gl->ptt_program.sampler0   = glGetUniformLocation(gl->ptt_program.id, "u_sampler0");
    gl->ptt_program.sampler1   = glGetUniformLocation(gl->ptt_program.id, "u_sampler1");
    gl->ptt_program.t          = glGetUniformLocation(gl->ptt_program.id, "u_t");
    gl->ptt_program.color_multiplier = glGetUniformLocation(gl->ptt_program.id, "u_color_multiplier");
    
    const char pta_shader[] =
#include "Shaders/pta_shader.glsl"
        ;
    gl->pta_program.id = load_shaders_by_text(pta_shader);
    gl->pta_program.position   = glGetAttribLocation (gl->pta_program.id, "a_position");
    gl->pta_program.tex_coords = glGetAttribLocation (gl->pta_program.id, "a_tex_coords");
    gl->pta_program.matrix     = glGetUniformLocation(gl->pta_program.id, "u_matrix");
    gl->pta_program.sampler    = glGetUniformLocation(gl->pta_program.id, "u_sampler");
    
    const char ptca_shader[] =
#include "Shaders/ptca_shader.glsl"
        ;
    gl->ptca_program.id = load_shaders_by_text(ptca_shader);
    gl->ptca_program.position   = glGetAttribLocation (gl->ptca_program.id, "a_position");
    gl->ptca_program.tex_coords = glGetAttribLocation (gl->ptca_program.id, "a_tex_coords");
    gl->ptca_program.color      = glGetAttribLocation (gl->ptca_program.id, "a_color");
    gl->ptca_program.matrix     = glGetUniformLocation(gl->ptca_program.id, "u_matrix");
    gl->ptca_program.sampler    = glGetUniformLocation(gl->ptca_program.id, "u_sampler");
    
    // We initialize the buffers
    glGenBuffers(1, &gl->level_buffer.buffer);
    glGenBuffers(1, &gl->level_changing_buffers[0].buffer);
    glGenBuffers(1, &gl->level_changing_buffers[1].buffer);
    glGenBuffers(1, &gl->goal_buffer.buffer);
    glGenBuffers(1, &gl->loading_buffer.buffer);
    glGenBuffers(1, &gl->player_buffer.buffer);
    glGenBuffers(1, &gl->particle_buffer.buffer);
    glGenBuffers(1, &gl->noise_buffer.buffer);
    glGenBuffers(1, &gl->text_buffer.buffer);
    glGenBuffers(1, &gl->ui_textures_buffer.buffer);
    glGenBuffers(1, &gl->tmp_buffer.buffer);
#if OS == OS_IOS
    glGenBuffers(1, &gl->buttons_buffer.buffer);
#endif
    
    check_openGL_error();
    // We initialize the vertex array objects
    glGenVertexArrays(1, &gl->level_vao);
    glBindVertexArray(gl->level_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->level_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    for(int i=0; i<2; i++){
        glGenVertexArrays(1, &gl->level_changing_vaos[i]);
        glBindVertexArray(gl->level_changing_vaos[i]);
        glEnableVertexAttribArray(gl->pca_program.position);
        glEnableVertexAttribArray(gl->pca_program.color);
        glBindBuffer(GL_ARRAY_BUFFER, gl->level_changing_buffers[0].buffer);
        glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
        glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
        check_openGL_error();
    }
    
    glGenVertexArrays(1, &gl->goal_vao);
    glBindVertexArray(gl->goal_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->goal_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->loading_vao);
    glBindVertexArray(gl->loading_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->loading_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->player_vao);
    glBindVertexArray(gl->player_vao);
    glEnableVertexAttribArray(gl->pt_program.position);
    glEnableVertexAttribArray(gl->pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->player_buffer.buffer);
    glVertexAttribPointer(gl->pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(gl->pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->particle_vao);
    glBindVertexArray(gl->particle_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->particle_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->noise_vao);
    glBindVertexArray(gl->noise_vao);
    glEnableVertexAttribArray(gl->ptt_program.position);
    glEnableVertexAttribArray(gl->ptt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->noise_buffer.buffer);
    glVertexAttribPointer(gl->ptt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PN), (void *)0);
    glVertexAttribPointer(gl->ptt_program.tex_coords, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PN), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->text_vao);
    glBindVertexArray(gl->text_vao);
    glEnableVertexAttribArray(gl->ptca_program.position);
    glEnableVertexAttribArray(gl->ptca_program.tex_coords);
    glEnableVertexAttribArray(gl->ptca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->text_buffer.buffer);
    glVertexAttribPointer(gl->ptca_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)0);
    glVertexAttribPointer(gl->ptca_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, t));
    glVertexAttribPointer(gl->ptca_program.color,      4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, c));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->ui_textures_vao);
    glBindVertexArray(gl->ui_textures_vao);
    glEnableVertexAttribArray(gl->pt_program.position);
    glEnableVertexAttribArray(gl->pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->ui_textures_buffer.buffer);
    glVertexAttribPointer(gl->pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(gl->pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &gl->tmp_vao);
    glBindVertexArray(gl->tmp_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->tmp_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
#if OS == OS_IOS
    glGenVertexArrays(1, &gl->buttons_vao);
    glBindVertexArray(gl->buttons_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->buttons_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT,         GL_FALSE, sizeof(Vertex_PCa), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex_PCa), (void *)(sizeof(Vec3)));
    check_openGL_error();
#endif
    
    glBindVertexArray(0);
    check_openGL_error();
    
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    char path[MAX_PATH_LENGTH];
    get_game_file_path("Textures/textures.png", path);
    load_texture(&texture, path);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    check_openGL_error();
    
    GLuint noise_texture;
    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    GLuint noise_texture_2;
    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &noise_texture_2);
    glBindTexture(GL_TEXTURE_2D, noise_texture_2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    change_window_size(game_state);
    
    //load_level_into_buffer(game_state, &gl->level_buffer, &gl->noise_buffer);
    
    check_openGL_error();
    
    /*
    // Init catalogs
    {
        void model_callback(void *);
        char path[MAX_PATH_LENGTH];
        get_game_file_path("Models/wall.model", path);
        models_info[0] = ModelInfo{"Models/wall.model", &static_models[0]};
        catalog_add(path, (void *)&models_info[0], &model_callback);
    }*/
}

/*void model_callback(void *info){
    get_static_model((const char *)((ModelInfo *)info)->path, ((ModelInfo *)info)->model);
}*/

#if MEASURE_RENDER_TIME
#include <time.h>
const int time_measures_size = 100;
int time_measures[time_measures_size];
#define start_time() clock_t __t0 = clock()
#define print_time(num) { clock_t t1 = clock(); time_measures[num] += t1-__t0; __t0 = t1; }
void print_render_time(){
    for(int i=0; i<time_measures_size; i++){
        int val = time_measures[i];
        if(i%10 == 0 && i > 0){
            for(int j=i-9; j<i; j++){
                val += time_measures[j];
            }
        }
        if(val != 0)
            printf("%i: %lg\n", i, (double)val/CLOCKS_PER_SEC);
    }
    for(int i=0; i<time_measures_size; i++){
        time_measures[i] = 0;
    }
}
#else
#define start_time()
#define print_time(num)
#endif

#define DONT_CHECK_ERRORS 0
#if DONT_CHECK_ERRORS
#undef check_openGL_error
#define check_openGL_error()
#endif

void draw_scene(GameState *game_state, bool should_redraw_level, bool should_redraw_state){
    start_time();
    
    GLObjects *gl = &game_state->gl_objects;
    PlayerSnapshot *rendered_player = &game_state->rendered_player;
    
    load_player_into_buffer(game_state, &gl->player_buffer); // This must be first, because it sets rendered_player
    load_particles_into_buffer(game_state, &gl->particle_buffer);
    load_goal_into_buffer(game_state, &gl->goal_buffer);
    static int drawn_level_state;
    if(should_redraw_level){
        load_level_into_buffer(game_state, &gl->level_buffer, &gl->noise_buffer);
        load_changing_level_into_buffer(&game_state->level, &gl->level_changing_buffers[0]);
        change_level_state(&game_state->level);
        load_changing_level_into_buffer(&game_state->level, &gl->level_changing_buffers[1]);
        change_level_state(&game_state->level);
        
        memcpy(&game_state->lagged_level, &game_state->next_lagged_level, sizeof(LaggedLevel));
    }else if(should_redraw_state){
        drawn_level_state = 1-drawn_level_state;
    }
    
    Mat4 final_matrix;
    Vec2 screen_size_in_level_tr;
    Vec2 screen_size_in_level_bl;
    if(gl->shown_level_size.y/gl->shown_level_size.x < gl->screen_size.y/gl->screen_size.x){
        // Sides of level touch sides of screen
        float scale_factor = 2.f/gl->shown_level_size.x;
        float other = scale_factor*gl->screen_size.x/gl->actual_screen_size.y;
        final_matrix = get_translation_matrix(Vec3(0.f, -screen_portion_of_ui_top+screen_portion_of_ui_bottom, 0.f)) * get_scale_matrix(scale_factor, other, 1.f) * get_translation_matrix(Vec3(gl->screen_translate.x, gl->screen_translate.y, 0.f));
        Vec2 ssil = Vec2(gl->shown_level_size.x/2.f, 1.f/other);
        screen_size_in_level_bl = -ssil - gl->screen_translate;
        screen_size_in_level_tr =  ssil - gl->screen_translate;
    }else{
        // Top and bottom of level touch top and bottom of screen
        float scale_factor = 2.f/gl->shown_level_size.y*gl->screen_size.y/gl->actual_screen_size.y;
        float other = scale_factor*gl->actual_screen_size.y/gl->screen_size.x;
        final_matrix = get_translation_matrix(Vec3(0.f, -screen_portion_of_ui_top+screen_portion_of_ui_bottom, 0.f)) * get_scale_matrix(other, scale_factor, 1.f) * get_translation_matrix(Vec3(gl->screen_translate.x, gl->screen_translate.y, 0.f));
        Vec2 ssil = Vec2(1.f/other, gl->shown_level_size.y/2.f);
        screen_size_in_level_bl = -ssil - gl->screen_translate;
        screen_size_in_level_tr =  ssil - gl->screen_translate;
    }
    
    
    Mat4 ui_matrix = get_translation_matrix(Vec3(-1.f, -1.f, 0.f)) * get_scale_matrix(2.f, 2.f/gl->actual_screen_ratio, 1.f);
    
    {
        const int texture_size = 32;
        const float freq = 4.f/60.f;
        static float time = 0.f;
        time += TIME_STEP/4.f;
        if(time > 0.f){
            static u8 rands[5][texture_size][texture_size];
            static bool fice = true;
            if(fice){
                for(int d=1, dc=0; dc<=5; d*=2, dc++){
                    int s = texture_size/d;
                    for(int i=0; i<s; i++){
                        for(int j=0; j<s; j++){
                            rands[dc][i][j] = rand()%8;
                        }
                    }
                }
                fice = false;
            }
            /*for(int t=0; t<1; t++){
             for(int d=1, dc=0; d<=4; d*=2, dc++){
             int s = texture_size/d;
             int i = rand()%s;
             int j = rand()%s;
             rands[dc][i][j] = rand()%10;
             }
             }*/
            
            GLubyte image_data[3*texture_size*texture_size];
            for(int k=0, kc=0; k<3*texture_size*texture_size; kc++){
                int i = kc/texture_size;
                int j = kc%texture_size;
                u8 r = rands[0][i][j] + rands[1][i/2][j/2] + rands[2][i/4][j/4] + rands[3][i/8][j/8] + rands[4][i/16][j/16];
                image_data[k++] = level_color.r;
                image_data[k++] = level_color.g;
                image_data[k++] = level_color.b+r;
            }
            glActiveTexture(GL_TEXTURE1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            time -= freq;
        }
    }{
        const int texture_size = 32;
        const float freq = 4.f/60.f;
        static float time = 0.f;
        time += TIME_STEP/4.f;
        if(time > 0.f){
            static u8 rands[5][texture_size][texture_size];
            static bool fice = true;
            if(fice){
                for(int d=1, dc=0; dc<=5; d*=2, dc++){
                    int s = texture_size/d;
                    for(int i=0; i<s; i++){
                        for(int j=0; j<s; j++){
                            rands[dc][i][j] = rand()%8;
                        }
                    }
                }
                fice = false;
            }
            /*for(int t=0; t<1; t++){
             for(int d=1, dc=0; d<=4; d*=2, dc++){
             int s = texture_size/d;
             int i = rand()%s;
             int j = rand()%s;
             rands[dc][i][j] = rand()%10;
             }
             }*/
            
            GLubyte image_data[3*texture_size*texture_size];
            for(int k=0, kc=0; k<3*texture_size*texture_size; kc++){
                int i = kc/texture_size;
                int j = kc%texture_size;
                u8 r = rands[0][i][j] + rands[1][i/2][j/2] + rands[2][i/4][j/4] + rands[3][i/8][j/8] + rands[4][i/16][j/16];
                image_data[k++] = level_color.r+r;
                image_data[k++] = level_color.g+r;
                image_data[k++] = level_color.b+r;
            }
            glActiveTexture(GL_TEXTURE3);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_size, texture_size, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            time -= freq;
        }
    }
    
    print_time(00);
    
    glClearColor(basic_color.r/255.f, basic_color.g/255.f, basic_color.b/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(gl->ptt_program.id);
    glUniformMatrix4fv(gl->ptt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    float t = 1.f;//-0.4f*MAX(MIN(rendered_player.cancel_next_level_in, 1.f), 0);
    glUniform3f(gl->ptt_program.color_multiplier, 1.f, t, t);
    
    if(gl->noise_buffer.count > 0){
        glUniform1i(gl->ptt_program.sampler0, 1);
        glUniform1i(gl->ptt_program.sampler1, 3);
        glUniform1f(gl->ptt_program.t, game_state->time);
        glBindVertexArray(gl->noise_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->noise_buffer.count);
        check_openGL_error();
    }
    
    glUseProgram(gl->pt_program.id);
    glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    glUniform3f(gl->pt_program.color_multiplier, 1.f, t, t);
    
    if(gl->player_buffer.count > 0){
        glUniform1i(gl->pt_program.sampler, 0);
        glBindVertexArray(gl->player_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->player_buffer.count);
        check_openGL_error();
    }
    
    if(gl->ui_textures_buffer.count > 0){
        glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(gl->ui_textures_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->ui_textures_buffer.count);
        check_openGL_error();
    }
    
    glUseProgram(gl->pca_program.id);
    glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    
    if(gl->level_buffer.count > 0){
        glBindVertexArray(gl->level_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_buffer.count);
        check_openGL_error();
    }
    if(gl->level_changing_buffers[drawn_level_state].count > 0){
        glBindVertexArray(gl->level_changing_vaos[drawn_level_state]);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_changing_buffers[drawn_level_state].count);
        check_openGL_error();
    }
    if(gl->goal_buffer.count > 0){
        glBindVertexArray(gl->goal_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->goal_buffer.count);
        check_openGL_error();
    }
    
    if(gl->particle_buffer.count > 0){
        glBindVertexArray(gl->particle_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->particle_buffer.count);
        check_openGL_error();
    }
    
    {
        start_temp_alloc();
        u32 vert_num = 0;
        
        float time = game_state->time;
        
        String time_string;
        time_string.allocator = &temporary_storage.allocator;
        char time_sc[30];
        if(time < 60.)
            sprintf(time_sc, "%.2lf ", time);
        else if(time < 3600.){
            int sec = (int)time;
            int min = sec/60;
            sprintf(time_sc, "%i:%05.2lf ", min, time-min*60.);
        }else{
            int sec = (int)time;
            int min = sec/60;
            int h   = min/60;
            sprintf(time_sc, "%i:%02i:%05.2lf ", h, min%60, time-min*60.);
        }
        time_string = time_sc;
        vert_num += text_vert_num(time_string);
        
        String lives_string;
        lives_string.allocator = &temporary_storage.allocator;
        char lives_sc[30];
        if(game_state->player_lives >= 0)
            sprintf(lives_sc, "   x%i", game_state->player_lives);
        else
            sprintf(lives_sc, "   x(%i)", game_state->player_lives);
        lives_string = lives_sc;
        vert_num += text_vert_num(lives_string);
        
        
        Vertex_PTCa *o_verts = (Vertex_PTCa *)temp_alloc(vert_num*sizeof(Vertex_PTCa));
        Vertex_PTCa *verts = o_verts;
        
        float spb = screen_portion_of_ui_bottom * gl->actual_screen_ratio;
        float spt = screen_portion_of_ui_top    * gl->actual_screen_ratio;
        
        float time_width;
        Vertex_PTCa *verts_time = verts + render_text_monospace(1.f, gl->actual_screen_ratio+0.2f*spt, -0.9f, FONT_QUALITY_64, time_string, verts, spt, &time_width, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP);
        while(verts < verts_time){
            verts->p.x -= time_width;
            verts++;
        }
        verts += render_text_monospace(0.f, gl->actual_screen_ratio+0.2f*spt, -0.9f, FONT_QUALITY_64, lives_string, verts, spt, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP);
        
        gl->text_buffer.count = vert_num;
        set_buffer_data_static(gl->text_buffer.buffer, o_verts, gl->text_buffer.count);
        
        end_temp_alloc();
    }
    
    if(gl->text_buffer.count > 0){
        glUseProgram(gl->ptca_program.id);
        glUniformMatrix4fv(gl->ptca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glUniform1i(gl->ptca_program.sampler, 2);
        
        glBindVertexArray(gl->text_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->text_buffer.count);
        check_openGL_error();
    }
    
#if OS == OS_IOS
    {
        float pun = 0.2*gl->actual_screen_ratio;
        float hpun = 0.5f*pun;
        float yun = 0.2*pun;
        const float _factor = 0.5f*(1.f-(0.6f*SQRT_3_2));
        float xun = _factor*pun;
        float margin = 0.02*gl->actual_screen_ratio;
        const RgbaColor bg = {100, 100, 100, 100};
        const RgbaColor fg = {200, 200, 200,  50};
        Vertex_PCa o_vertices[] = {
            {Vec3(margin+0.f, margin+0.f, -0.9f), bg},
            {Vec3(margin+pun, margin+0.f, -0.9f), bg},
            {Vec3(margin+0.f, margin+pun, -0.9f), bg},
            {Vec3(margin+0.f, margin+pun, -0.9f), bg},
            {Vec3(margin+pun, margin+0.f, -0.9f), bg},
            {Vec3(margin+pun, margin+pun, -0.9f), bg},
            
            {Vec3(margin+xun,     margin+hpun,    -1.f), fg},
            {Vec3(margin+pun-xun, margin+yun,     -1.f), fg},
            {Vec3(margin+pun-xun, margin+pun-yun, -1.f), fg},
            
            {Vec3(2.f*margin+1.f*pun, margin+0.f, -0.9f), bg},
            {Vec3(2.f*margin+2.f*pun, margin+0.f, -0.9f), bg},
            {Vec3(2.f*margin+1.f*pun, margin+pun, -0.9f), bg},
            {Vec3(2.f*margin+1.f*pun, margin+pun, -0.9f), bg},
            {Vec3(2.f*margin+2.f*pun, margin+0.f, -0.9f), bg},
            {Vec3(2.f*margin+2.f*pun, margin+pun, -0.9f), bg},
            
            {Vec3(2.f*margin+2.f*pun-xun, margin+hpun,    -1.f), fg},
            {Vec3(2.f*margin+1.f*pun+xun, margin+yun,     -1.f), fg},
            {Vec3(2.f*margin+1.f*pun+xun, margin+pun-yun, -1.f), fg},
            
            {Vec3(1.f-margin,     margin+0.f, -1.f), bg},
            {Vec3(1.f-margin-pun, margin+0.f, -1.f), bg},
            {Vec3(1.f-margin,     margin+pun, -1.f), bg},
            {Vec3(1.f-margin,     margin+pun, -1.f), bg},
            {Vec3(1.f-margin-pun, margin+0.f, -1.f), bg},
            {Vec3(1.f-margin-pun, margin+pun, -1.f), bg},
        };
        for(int k=0; k<3; k++){
            if(player.keys[k]){
                int im = 9*k;
                for(int i=0; i<6; i++){
                    o_vertices[im+i].c = fg;
                }
                if(k < 2){
                    for(int i=6; i<9; i++){
                        o_vertices[im+i].c = bg;
                    }
                }
            }
        }
        gl->buttons_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(gl->buttons_buffer.buffer, o_vertices, gl->buttons_buffer.count);
        
        glUseProgram(gl->pca_program.id);
        glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(gl->buttons_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->buttons_buffer.count);
        check_openGL_error();
    }
#endif
}

void change_window_size(GameState *game_state){
    GLObjects *gl = &game_state->gl_objects;
    
    gl->actual_screen_size.x = (f32)window_size.x;
    gl->actual_screen_size.y = (f32)window_size.y;
    gl->screen_size.x = gl->actual_screen_size.x;
    gl->screen_size.y = (1.f - screen_portion_of_ui_top - screen_portion_of_ui_bottom)*gl->actual_screen_size.y;
    gl->actual_screen_ratio = gl->actual_screen_size.y / gl->actual_screen_size.x;
    if(window_size.x >= window_size.y){
        gl->screen_ratio.x = (float) window_size.y / window_size.x;
        gl->screen_ratio.y = 1.f;
    }else{
        gl->screen_ratio.x = 1.f;
        gl->screen_ratio.y = (float) window_size.x / window_size.x;
    }
    glViewport(0, 0, window_size.x, window_size.y);
    glScissor(0, 0, window_size.x, window_size.y);
    
    {
        const float texture_size = 256.f;
        const float outer_picture_size = 32.f;
        const float inner_picture_size = 30.f;
        const float picture_margin = 1.f;
        const float current_frame_x = 7.f;
        const float current_frame_y = 0.f;
        float xmargin = 0.9f*screen_portion_of_ui_top*gl->actual_screen_ratio;
        float ymargin = (1.f-0.9f*screen_portion_of_ui_top)*gl->actual_screen_ratio;
        float pun = 0.8f*screen_portion_of_ui_top*gl->actual_screen_ratio;
        Vec2 t00, t01, t10, t11;
        t00 = Vec2(current_frame_x*outer_picture_size+picture_margin, current_frame_y*outer_picture_size+picture_margin)/texture_size;
        t10 = t00+Vec2(inner_picture_size/texture_size, 0.f);
        t01 = t00+Vec2(0.f, inner_picture_size/texture_size);
        t11 = t10+Vec2(0.f, inner_picture_size/texture_size);
        Vertex_PT o_vertices[] = {
            {Vec3(xmargin+0.f, ymargin+0.f, -0.9f), t00},
            {Vec3(xmargin+pun, ymargin+0.f, -0.9f), t10},
            {Vec3(xmargin+0.f, ymargin+pun, -0.9f), t01},
            {Vec3(xmargin+0.f, ymargin+pun, -0.9f), t01},
            {Vec3(xmargin+pun, ymargin+0.f, -0.9f), t10},
            {Vec3(xmargin+pun, ymargin+pun, -0.9f), t11},
        };
        gl->ui_textures_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(gl->ui_textures_buffer.buffer, o_vertices, ArrayCount(o_vertices));
    }
    {
        RgbaColor c = {155, 0, 0, 255};
        float y0 = screen_portion_of_ui_bottom * gl->actual_screen_ratio;
        float y1 = screen_portion_of_ui_top    * gl->actual_screen_ratio;
        Vertex_PCa o_vertices[] = {
            {Vec3(0.f, 0.f, -0.8f), c},
            {Vec3(1.f, 0.f, -0.8f), c},
            {Vec3(0.f, y0,   -0.8f), c},
            {Vec3(0.f, y0,   -0.8f), c},
            {Vec3(1.f, 0.f, -0.8f), c},
            {Vec3(1.f, y0,   -0.8f), c},
            
            {Vec3(0.f, gl->actual_screen_ratio,    -0.8f), c},
            {Vec3(1.f, gl->actual_screen_ratio,    -0.8f), c},
            {Vec3(0.f, gl->actual_screen_ratio-y1, -0.8f), c},
            {Vec3(0.f, gl->actual_screen_ratio-y1, -0.8f), c},
            {Vec3(1.f, gl->actual_screen_ratio,    -0.8f), c},
            {Vec3(1.f, gl->actual_screen_ratio-y1, -0.8f), c},
        };
        gl->tmp_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(gl->tmp_buffer.buffer, o_vertices, ArrayCount(o_vertices));
    }
}