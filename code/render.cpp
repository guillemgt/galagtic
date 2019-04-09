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
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !OPENGL_ES
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.02f);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif
    
    glEnable(GL_SCISSOR_TEST);
    
    
    
    GLObjects *gl = &game_state->gl_objects;
    // We read the wall shaders
    const char pca_shader_v[] =
#if !OPENGL_ES
#include "Shaders/pca_shader_v.glsl"
#else
#include "Shaders/pca_shader_v_es.glsl"
#endif
        ;
    const char pca_shader_f[] =
#if !OPENGL_ES
#include "Shaders/pca_shader_f.glsl"
#else
#include "Shaders/pca_shader_f_es.glsl"
#endif
        ;
    gl->pca_program.id = ggtgl_load_shaders_by_text(pca_shader_v, pca_shader_f);
    gl->pca_program.position = glGetAttribLocation (gl->pca_program.id, "a_position");
    gl->pca_program.color    = glGetAttribLocation (gl->pca_program.id, "a_color");
    gl->pca_program.matrix   = glGetUniformLocation(gl->pca_program.id, "u_matrix");
    
    const char pt_shader_v[] =
#if !OPENGL_ES
#include "Shaders/pt_shader_v.glsl"
#else
#include "Shaders/pt_shader_v_es.glsl"
#endif
        ;
    const char pt_shader_f[] =
#if !OPENGL_ES
#include "Shaders/pt_shader_f.glsl"
#else
#include "Shaders/pt_shader_f_es.glsl"
#endif
        ;
    gl->pt_program.id = ggtgl_load_shaders_by_text(pt_shader_v, pt_shader_f);
    gl->pt_program.position   = glGetAttribLocation (gl->pt_program.id, "a_position");
    gl->pt_program.tex_coords = glGetAttribLocation (gl->pt_program.id, "a_tex_coords");
    gl->pt_program.matrix     = glGetUniformLocation(gl->pt_program.id, "u_matrix");
    gl->pt_program.sampler    = glGetUniformLocation(gl->pt_program.id, "u_sampler");
    gl->pt_program.color_multiplier = glGetUniformLocation(gl->pt_program.id, "u_color_multiplier");
    
    const char pta_shader_v[] =
#if !OPENGL_ES
#include "Shaders/pta_shader_v.glsl"
#else
#include "Shaders/pta_shader_v_es.glsl"
#endif
        ;
    const char pta_shader_f[] =
#if !OPENGL_ES
#include "Shaders/pta_shader_f.glsl"
#else
#include "Shaders/pta_shader_f_es.glsl"
#endif
        ;
    gl->pta_program.id = ggtgl_load_shaders_by_text(pta_shader_v, pta_shader_f);
    gl->pta_program.position   = glGetAttribLocation (gl->pta_program.id, "a_position");
    gl->pta_program.tex_coords = glGetAttribLocation (gl->pta_program.id, "a_tex_coords");
    gl->pta_program.matrix     = glGetUniformLocation(gl->pta_program.id, "u_matrix");
    gl->pta_program.sampler    = glGetUniformLocation(gl->pta_program.id, "u_sampler");
    
    const char ptca_shader_v[] =
#if !OPENGL_ES
#include "Shaders/ptca_shader_v.glsl"
#else
#include "Shaders/ptca_shader_v_es.glsl"
#endif
        ;
    const char ptca_shader_f[] =
#if !OPENGL_ES
#include "Shaders/ptca_shader_f.glsl"
#else
#include "Shaders/ptca_shader_f_es.glsl"
#endif
        ;
    gl->ptca_program.id = ggtgl_load_shaders_by_text(ptca_shader_v, ptca_shader_f);
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
    glGenBuffers(1, &gl->fade_buffer.buffer);
    glGenBuffers(1, &gl->text_buffer.buffer);
    glGenBuffers(1, &gl->ui_textures_buffer.buffer);
    glGenBuffers(1, &gl->planet_buffer.buffer);
#if OS == OS_IOS
    glGenBuffers(1, &gl->buttons_buffer.buffer);
#endif
    
    ggtgl_check_error();
    // We initialize the vertex array objects
    glGenVertexArrays(1, &gl->level_vao);
    glBindVertexArray(gl->level_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->level_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    for(int i=0; i<2; i++){
        glGenVertexArrays(1, &gl->level_changing_vaos[i]);
        glBindVertexArray(gl->level_changing_vaos[i]);
        glEnableVertexAttribArray(gl->pca_program.position);
        glEnableVertexAttribArray(gl->pca_program.color);
        glBindBuffer(GL_ARRAY_BUFFER, gl->level_changing_buffers[i].buffer);
        glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
        glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
        ggtgl_check_error();
    }
    
    glGenVertexArrays(1, &gl->goal_vao);
    glBindVertexArray(gl->goal_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->goal_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    glGenVertexArrays(1, &gl->loading_vao);
    glBindVertexArray(gl->loading_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->loading_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    glGenVertexArrays(1, &gl->player_vao);
    glBindVertexArray(gl->player_vao);
    glEnableVertexAttribArray(gl->pt_program.position);
    glEnableVertexAttribArray(gl->pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->player_buffer.buffer);
    glVertexAttribPointer(gl->pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(gl->pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    glGenVertexArrays(1, &gl->particle_vao);
    glBindVertexArray(gl->particle_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->particle_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    glGenVertexArrays(1, &gl->fade_vao);
    glBindVertexArray(gl->fade_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->fade_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    
    glGenVertexArrays(1, &gl->text_vao);
    glBindVertexArray(gl->text_vao);
    glEnableVertexAttribArray(gl->ptca_program.position);
    glEnableVertexAttribArray(gl->ptca_program.tex_coords);
    glEnableVertexAttribArray(gl->ptca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->text_buffer.buffer);
    glVertexAttribPointer(gl->ptca_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)0);
    glVertexAttribPointer(gl->ptca_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, t));
    glVertexAttribPointer(gl->ptca_program.color,      4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, c));
    ggtgl_check_error();
    
    glGenVertexArrays(1, &gl->ui_textures_vao);
    glBindVertexArray(gl->ui_textures_vao);
    glEnableVertexAttribArray(gl->pt_program.position);
    glEnableVertexAttribArray(gl->pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->ui_textures_buffer.buffer);
    glVertexAttribPointer(gl->pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(gl->pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
#if OS == OS_IOS
    glGenVertexArrays(1, &gl->buttons_vao);
    glBindVertexArray(gl->buttons_vao);
    glEnableVertexAttribArray(gl->pca_program.position);
    glEnableVertexAttribArray(gl->pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, gl->buttons_buffer.buffer);
    glVertexAttribPointer(gl->pca_program.position, 3, GL_FLOAT,         GL_FALSE, sizeof(Vertex_PCa), (void *)0);
    glVertexAttribPointer(gl->pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex_PCa), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
#endif
    
    glGenVertexArrays(1, &gl->planet_vao);
    glBindVertexArray(gl->planet_vao);
    glEnableVertexAttribArray(gl->pt_program.position);
    glEnableVertexAttribArray(gl->pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, gl->planet_buffer.buffer);
    glVertexAttribPointer(gl->pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(gl->pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    ggtgl_check_error();
    
    glBindVertexArray(0);
    ggtgl_check_error();
    
    glActiveTexture(GL_TEXTURE0);
    GLuint texture;
    char path[MAX_PATH_LENGTH];
    ggtp_program_file_path("Textures/textures.png", path);
    ggtgl_load_texture(&texture, path);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ggtgl_check_error();
    
    glActiveTexture(GL_TEXTURE1);
    GLuint texture2;
    ggtp_program_file_path("Textures/planet.png", path);
    ggtgl_load_texture(&texture2, path);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ggtgl_check_error();
    
    change_window_size(game_state, (Vec2)window_size);
    
    //load_level_into_buffer(game_state, &gl->level_buffer, &gl->noise_buffer);
    
    ggtgl_check_error();
    
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
#undef ggtgl_check_error
#define ggtgl_check_error()
#endif

void draw_scene(GameState *game_state, bool should_redraw_level, bool draw_ui_and_player){
    start_time();
    
    GLObjects *gl = &game_state->gl_objects;
    PlayerSnapshot *rendered_player = &game_state->rendered_player;
    
    find_snapshot_to_render(game_state);
    load_player_into_buffer(game_state, &gl->player_buffer); // This must be first, because it sets rendered_player
    load_particles_into_buffer(game_state, &gl->particle_buffer);
    load_goal_into_buffer(game_state, &gl->goal_buffer);
    if(should_redraw_level){
        load_level_into_buffer(game_state, &gl->level_buffer);
        load_changing_level_into_buffer(&game_state->level, &gl->level_changing_buffers[0]);
        change_level_state(&game_state->level);
        load_changing_level_into_buffer(&game_state->level, &gl->level_changing_buffers[1]);
        change_level_state(&game_state->level);
        
        memcpy(&game_state->lagged_level, &game_state->next_lagged_level, sizeof(LaggedLevel));
        gl->drawn_level_state = 0;
        load_planet_background(&game_state->level, &gl->planet_buffer);
    }
    
    float scale_to_match_x = 2.f/gl->shown_level_size.x;
    float scale_to_match_y = 2.f/gl->shown_level_size.y * gl->actual_screen_ratio;
    float scale_to_match_y_with_ui = 2.f/(gl->shown_level_size.y-1.f) * (gl->screen_size.y / gl->actual_screen_size.x);
    float min_scale = fminf(scale_to_match_x, fminf(scale_to_match_y, scale_to_match_y_with_ui));
    
    Mat4 final_matrix = get_scale_matrix(min_scale, min_scale / gl->actual_screen_ratio, 1.f) * get_translation_matrix(Vec3(gl->screen_translate.x, gl->screen_translate.y, 0.f));
    
    if(min_scale == scale_to_match_y_with_ui)
        final_matrix = get_translation_matrix(Vec3(0.f, -0.5f*screen_portion_of_ui_top, 0.f)) * final_matrix;
    
    Mat4 ui_matrix = get_translation_matrix(Vec3(-1.f, -1.f, 0.f)) * get_scale_matrix(2.f, 2.f/gl->actual_screen_ratio, 1.f);
    
    print_time(00);
    
    glClearColor(basic_color.r/255.f, basic_color.g/255.f, basic_color.b/255.f, 1.f);
    //glClearColor(0.f, 0.f, 0.f, 1.f);
    //glClearColor(45.f/255.f, 45.f/255.f, 45.f/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glUseProgram(gl->pt_program.id);
    glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    glUniform3f(gl->pt_program.color_multiplier, 1.f, 1.f, 1.f);
    
    if(gl->planet_buffer.count > 0){
        glUniform1i(gl->pt_program.sampler, 1);
        glBindVertexArray(gl->planet_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->planet_buffer.count);
        ggtgl_check_error();
    }
    
    if(draw_ui_and_player && gl->player_buffer.count > 0){
        glUniform1i(gl->pt_program.sampler, 0);
        glBindVertexArray(gl->player_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->player_buffer.count);
        ggtgl_check_error();
    }
    
    if(draw_ui_and_player && gl->ui_textures_buffer.count > 0){
        glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(gl->ui_textures_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->ui_textures_buffer.count);
        ggtgl_check_error();
    }
    
    glUseProgram(gl->pca_program.id);
    glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    
    if(gl->level_buffer.count > 0){
        glBindVertexArray(gl->level_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_buffer.count);
        ggtgl_check_error();
    }
    if(gl->level_changing_buffers[gl->drawn_level_state].count > 0){
        glBindVertexArray(gl->level_changing_vaos[gl->drawn_level_state]);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_changing_buffers[gl->drawn_level_state].count);
        ggtgl_check_error();
    }
    if(gl->goal_buffer.count > 0){
        glBindVertexArray(gl->goal_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->goal_buffer.count);
        ggtgl_check_error();
    }
    
    if(gl->particle_buffer.count > 0){
        glBindVertexArray(gl->particle_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->particle_buffer.count);
        ggtgl_check_error();
    }
    
    {
        start_temp_alloc();
        u32 vert_num = 0;
        
        float time = game_state->time - game_state->time_started_counting;
        
        char time_sc[30];
        if(time < 0.f)
            sprintf(time_sc, "");
        else if(time < 60.)
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
        vert_num += text_vert_num(time_sc);
        
        char lives_sc[30];
        if(game_state->player_lives >= 0)
            sprintf(lives_sc, "   x%i", game_state->player_lives);
        else
            sprintf(lives_sc, "   x(%i)", game_state->player_lives);
        vert_num += text_vert_num(lives_sc);
        
        
        Vertex_PTCa *o_verts = (Vertex_PTCa *)talloc(vert_num*sizeof(Vertex_PTCa));
        Vertex_PTCa *verts = o_verts;
        
        float spb = screen_portion_of_ui_bottom * gl->actual_screen_ratio;
        float spt = screen_portion_of_ui_top    * gl->actual_screen_ratio;
        
        float time_width;
        Vertex_PTCa *verts_time = verts + render_text(1.f, gl->actual_screen_ratio+0.2f*spt, -0.7f, FONT_QUALITY_64, time_sc, (mt_Vertex_PTCa *)verts, spt, &time_width, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP | TEXT_MONOSPACE);
        while(verts < verts_time){
            verts->p.x -= time_width;
            verts++;
        }
        verts += render_text(0.f, gl->actual_screen_ratio+0.2f*spt, -0.7f, FONT_QUALITY_64, lives_sc, (mt_Vertex_PTCa *)verts, spt, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP | TEXT_MONOSPACE);
        
        gl->text_buffer.count = vert_num;
        ggtgl_set_buffer_data(gl->text_buffer.buffer, o_verts, gl->text_buffer.count, GL_STATIC_DRAW);
        
        end_temp_alloc();
    }
    
    if(draw_ui_and_player && gl->text_buffer.count > 0){
        glUseProgram(gl->ptca_program.id);
        glUniformMatrix4fv(gl->ptca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glUniform1i(gl->ptca_program.sampler, 2);
        
        glBindVertexArray(gl->text_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->text_buffer.count);
        ggtgl_check_error();
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
        const float z = -0.8f;
        const float zp = -0.81f;
        Vertex_PCa o_vertices[] = {
            {Vec3(margin+0.f, margin+0.f, z), bg},
            {Vec3(margin+pun, margin+0.f, z), bg},
            {Vec3(margin+0.f, margin+pun, z), bg},
            {Vec3(margin+0.f, margin+pun, z), bg},
            {Vec3(margin+pun, margin+0.f, z), bg},
            {Vec3(margin+pun, margin+pun, z), bg},
            
            {Vec3(margin+xun,     margin+hpun,    zp), fg},
            {Vec3(margin+pun-xun, margin+yun,     zp), fg},
            {Vec3(margin+pun-xun, margin+pun-yun, zp), fg},
            
            {Vec3(2.f*margin+1.f*pun, margin+0.f, z), bg},
            {Vec3(2.f*margin+2.f*pun, margin+0.f, z), bg},
            {Vec3(2.f*margin+1.f*pun, margin+pun, z), bg},
            {Vec3(2.f*margin+1.f*pun, margin+pun, z), bg},
            {Vec3(2.f*margin+2.f*pun, margin+0.f, z), bg},
            {Vec3(2.f*margin+2.f*pun, margin+pun, z), bg},
            
            {Vec3(2.f*margin+2.f*pun-xun, margin+hpun,    zp), fg},
            {Vec3(2.f*margin+1.f*pun+xun, margin+yun,     zp), fg},
            {Vec3(2.f*margin+1.f*pun+xun, margin+pun-yun, zp), fg},
            
            {Vec3(1.f-margin,     margin+0.f, zp), bg},
            {Vec3(1.f-margin-pun, margin+0.f, zp), bg},
            {Vec3(1.f-margin,     margin+pun, zp), bg},
            {Vec3(1.f-margin,     margin+pun, zp), bg},
            {Vec3(1.f-margin-pun, margin+0.f, zp), bg},
            {Vec3(1.f-margin-pun, margin+pun, zp), bg},
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
        ggtgl_check_error();
    }
#endif
}

void change_window_size(GameState *game_state, Vec2 size){
    GLObjects *gl = &game_state->gl_objects;
    
    gl->actual_screen_size = size;
    gl->screen_size.x = gl->actual_screen_size.x;
    gl->screen_size.y = (1.f - screen_portion_of_ui_top - screen_portion_of_ui_bottom)*gl->actual_screen_size.y;
    gl->actual_screen_ratio = gl->actual_screen_size.y / gl->actual_screen_size.x;
    if(size.x >= size.y){
        gl->screen_ratio.x = (float) window_size.y / window_size.x;
        gl->screen_ratio.y = 1.f;
    }else{
        gl->screen_ratio.x = 1.f;
        gl->screen_ratio.y = (float) window_size.x / window_size.x;
    }
    glViewport(0, 0, (int)size.x, (int)size.y);
    glScissor( 0, 0, (int)size.x, (int)size.y);
    
    {
        const float texture_size = 2048.f;
        const float outer_picture_size = 266.f;
        const float inner_picture_size = 256.f;
        const float picture_margin = 5.f;
        const int current_frame_x = 2;
        const int current_frame_y = 4;
        Vec2 t01 = Vec2(current_frame_x*outer_picture_size+picture_margin, texture_size-(current_frame_y*outer_picture_size+picture_margin))/texture_size;
        Vec2 t11 = t01+Vec2(inner_picture_size/texture_size, 0.f);
        Vec2 t00 = t01+Vec2(0.f, -inner_picture_size/texture_size);
        Vec2 t10 = t11+Vec2(0.f, -inner_picture_size/texture_size);
        
        float xmargin = 0.9f*screen_portion_of_ui_top*gl->actual_screen_ratio;
        float ymargin = (1.f-0.9f*screen_portion_of_ui_top)*gl->actual_screen_ratio;
        float pun = 0.8f*screen_portion_of_ui_top*gl->actual_screen_ratio;
        
        Vertex_PT o_vertices[] = {
            {Vec3(xmargin+0.f, ymargin+0.f, -0.9f), t00},
            {Vec3(xmargin+pun, ymargin+0.f, -0.9f), t10},
            {Vec3(xmargin+0.f, ymargin+pun, -0.9f), t01},
            {Vec3(xmargin+0.f, ymargin+pun, -0.9f), t01},
            {Vec3(xmargin+pun, ymargin+0.f, -0.9f), t10},
            {Vec3(xmargin+pun, ymargin+pun, -0.9f), t11},
        };
        gl->ui_textures_buffer.count = ArraySize(o_vertices);
        ggtgl_set_buffer_data(gl->ui_textures_buffer.buffer, o_vertices, ArraySize(o_vertices), GL_DYNAMIC_DRAW);
    }
}
