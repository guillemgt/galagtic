#include "basecode/basecode.hpp"
#include "basecode/os.hpp"

#include "render.hpp"
#include "player.hpp"
#include "main.hpp"

struct {
    GLuint id;
    GLuint position, color, matrix;
} pca_program;

struct {
    GLuint id;
    GLuint position, tex_coords, matrix, sampler, color_multiplier;
} pt_program;

struct {
    GLuint id;
    GLuint position, tex_coords, matrix, sampler0, sampler1, t, color_multiplier;
} ptt_program;

struct {
    GLuint id;
    GLuint position, tex_coords, matrix, sampler;
} pta_program;

struct {
    GLuint id;
    GLuint position, tex_coords, color, matrix, sampler;
} ptca_program;

BufferAndCount level_buffer;
BufferAndCount level_changing_buffer;
BufferAndCount level_moving_buffer;
BufferAndCount goal_buffer;
BufferAndCount loading_buffer;
BufferAndCount player_buffer;
BufferAndCount particle_buffer;
BufferAndCount noise_buffer;
BufferAndCount text_buffer;
BufferAndCount ui_textures_buffer;
BufferAndCount tmp_buffer;
#if OS == OS_IOS
BufferAndCount buttons_buffer;
#endif


GLuint level_vao;
GLuint level_changing_vao;
GLuint level_moving_vao;
GLuint goal_vao;
GLuint loading_vao;
GLuint player_vao;
GLuint particle_vao;
GLuint noise_vao;
GLuint text_vao;
GLuint ui_textures_vao;
GLuint tmp_vao;
#if OS == OS_IOS
GLuint buttons_vao;
#endif

Vec2 screen_ratio;
Vec2 screen_scale;
Vec2 screen_translate;
Vec2 shown_level_size;
Vec2 level_size;
Vec2 screen_size;
Vec2 actual_screen_size;
float actual_screen_ratio;
extern Vec2i window_size;


/*struct ModelInfo {
    char path[MAX_PATH_LENGTH];
    StaticModel *model;
};
ModelInfo models_info[10];
StaticModel static_models[10];*/

void init_openGL(){
    // We read the wall shaders
    const char wall_shader[] =
#include "Shaders/pca_shader.glsl"
    ;
    pca_program.id = load_shaders_by_text(wall_shader);
    pca_program.position = glGetAttribLocation (pca_program.id, "a_position");
    pca_program.color    = glGetAttribLocation (pca_program.id, "a_color");
    pca_program.matrix   = glGetUniformLocation(pca_program.id, "u_matrix");
    
    const char pt_shader[] =
#include "Shaders/pt_shader.glsl"
    ;
    pt_program.id = load_shaders_by_text(pt_shader);
    pt_program.position   = glGetAttribLocation (pt_program.id, "a_position");
    pt_program.tex_coords = glGetAttribLocation (pt_program.id, "a_tex_coords");
    pt_program.matrix     = glGetUniformLocation(pt_program.id, "u_matrix");
    pt_program.sampler    = glGetUniformLocation(pt_program.id, "u_sampler");
    pt_program.color_multiplier = glGetUniformLocation(pt_program.id, "u_color_multiplier");
    
    const char ptt_shader[] =
#include "Shaders/ptt_shader.glsl"
    ;
    ptt_program.id = load_shaders_by_text(ptt_shader);
    ptt_program.position   = glGetAttribLocation (ptt_program.id, "a_position");
    ptt_program.tex_coords = glGetAttribLocation (ptt_program.id, "a_tex_coords");
    ptt_program.matrix     = glGetUniformLocation(ptt_program.id, "u_matrix");
    ptt_program.sampler0   = glGetUniformLocation(ptt_program.id, "u_sampler0");
    ptt_program.sampler1   = glGetUniformLocation(ptt_program.id, "u_sampler1");
    ptt_program.t          = glGetUniformLocation(ptt_program.id, "u_t");
    ptt_program.color_multiplier = glGetUniformLocation(ptt_program.id, "u_color_multiplier");
    
    const char pta_shader[] =
#include "Shaders/pta_shader.glsl"
    ;
    pta_program.id = load_shaders_by_text(pta_shader);
    pta_program.position   = glGetAttribLocation (pta_program.id, "a_position");
    pta_program.tex_coords = glGetAttribLocation (pta_program.id, "a_tex_coords");
    pta_program.matrix     = glGetUniformLocation(pta_program.id, "u_matrix");
    pta_program.sampler    = glGetUniformLocation(pta_program.id, "u_sampler");
    
    const char ptca_shader[] =
#include "Shaders/ptca_shader.glsl"
    ;
    ptca_program.id = load_shaders_by_text(ptca_shader);
    ptca_program.position   = glGetAttribLocation (ptca_program.id, "a_position");
    ptca_program.tex_coords = glGetAttribLocation (ptca_program.id, "a_tex_coords");
    ptca_program.color      = glGetAttribLocation (ptca_program.id, "a_color");
    ptca_program.matrix     = glGetUniformLocation(ptca_program.id, "u_matrix");
    ptca_program.sampler    = glGetUniformLocation(ptca_program.id, "u_sampler");
    
    // We initialize the buffers
    glGenBuffers(1, &level_buffer.buffer);
    glGenBuffers(1, &level_changing_buffer.buffer);
    glGenBuffers(1, &level_moving_buffer.buffer);
    glGenBuffers(1, &goal_buffer.buffer);
    glGenBuffers(1, &loading_buffer.buffer);
    glGenBuffers(1, &player_buffer.buffer);
    glGenBuffers(1, &particle_buffer.buffer);
    glGenBuffers(1, &noise_buffer.buffer);
    glGenBuffers(1, &text_buffer.buffer);
    glGenBuffers(1, &ui_textures_buffer.buffer);
    glGenBuffers(1, &tmp_buffer.buffer);
#if OS == OS_IOS
    glGenBuffers(1, &buttons_buffer.buffer);
#endif
    
    check_openGL_error();
    // We initialize the vertex array objects
    glGenVertexArrays(1, &level_vao);
    glBindVertexArray(level_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, level_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &level_changing_vao);
    glBindVertexArray(level_changing_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, level_changing_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &level_moving_vao);
    glBindVertexArray(level_moving_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, level_moving_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &goal_vao);
    glBindVertexArray(goal_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, goal_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &loading_vao);
    glBindVertexArray(loading_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, loading_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &player_vao);
    glBindVertexArray(player_vao);
    glEnableVertexAttribArray(pt_program.position);
    glEnableVertexAttribArray(pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, player_buffer.buffer);
    glVertexAttribPointer(pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &particle_vao);
    glBindVertexArray(particle_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, particle_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &noise_vao);
    glBindVertexArray(noise_vao);
    glEnableVertexAttribArray(ptt_program.position);
    glEnableVertexAttribArray(ptt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, noise_buffer.buffer);
    glVertexAttribPointer(ptt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PN), (void *)0);
    glVertexAttribPointer(ptt_program.tex_coords, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PN), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &text_vao);
    glBindVertexArray(text_vao);
    glEnableVertexAttribArray(ptca_program.position);
    glEnableVertexAttribArray(ptca_program.tex_coords);
    glEnableVertexAttribArray(ptca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, text_buffer.buffer);
    glVertexAttribPointer(ptca_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)0);
    glVertexAttribPointer(ptca_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, t));
    glVertexAttribPointer(ptca_program.color,      4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PTCa), (void *)offsetof(Vertex_PTCa, c));
    check_openGL_error();

    glGenVertexArrays(1, &ui_textures_vao);
    glBindVertexArray(ui_textures_vao);
    glEnableVertexAttribArray(pt_program.position);
    glEnableVertexAttribArray(pt_program.tex_coords);
    glBindBuffer(GL_ARRAY_BUFFER, ui_textures_buffer.buffer);
    glVertexAttribPointer(pt_program.position,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)0);
    glVertexAttribPointer(pt_program.tex_coords, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_PT), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
    glGenVertexArrays(1, &tmp_vao);
    glBindVertexArray(tmp_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, tmp_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_PC), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex_PC), (void *)(sizeof(Vec3)));
    check_openGL_error();
    
#if OS == OS_IOS
    glGenVertexArrays(1, &buttons_vao);
    glBindVertexArray(buttons_vao);
    glEnableVertexAttribArray(pca_program.position);
    glEnableVertexAttribArray(pca_program.color);
    glBindBuffer(GL_ARRAY_BUFFER, buttons_buffer.buffer);
    glVertexAttribPointer(pca_program.position, 3, GL_FLOAT,         GL_FALSE, sizeof(Vertex_PCa), (void *)0);
    glVertexAttribPointer(pca_program.color,    4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(Vertex_PCa), (void *)(sizeof(Vec3)));
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
    
    change_window_size();
    
    load_level_into_buffer(&level_buffer, &noise_buffer);
    load_level_into_buffer(&level_moving_buffer, nullptr);
    
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

void draw_scene(){
    start_time();
    load_player_into_buffer(&player_buffer); // This must be first, because it sets rendered_player
    load_particles_into_buffer(&particle_buffer);
    load_goal_into_buffer(&goal_buffer, &loading_buffer);
    if(should_new_level){
        load_level_into_buffer(&level_buffer, &noise_buffer);
        load_level_into_buffer(&level_moving_buffer, nullptr);
        load_changing_level_into_buffer(&level_changing_buffer);
    }else if(should_update_level){
        load_changing_level_into_buffer(&level_changing_buffer);
    }
    
    Mat4 final_matrix;
    Vec2 screen_size_in_level_tr;
    Vec2 screen_size_in_level_bl;
    if(shown_level_size.y/shown_level_size.x < screen_size.y/screen_size.x){
        // Sides of level touch sides of screen
        float scale_factor = 2.f/shown_level_size.x;
        float other = scale_factor*screen_size.x/actual_screen_size.y;
        final_matrix = get_translation_matrix(Vec3(0.f, -screen_portion_of_ui_top+screen_portion_of_ui_bottom, 0.f)) * get_scale_matrix(scale_factor, other, 1.f) * get_translation_matrix(Vec3(screen_translate.x, screen_translate.y, 0.f));
        Vec2 ssil = Vec2(shown_level_size.x/2.f, 1.f/other);
        screen_size_in_level_bl = -ssil - screen_translate;
        screen_size_in_level_tr =  ssil - screen_translate;
    }else{
        // Top and bottom of level touch top and bottom of screen
        float scale_factor = 2.f/shown_level_size.y*screen_size.y/actual_screen_size.y;
        float other = scale_factor*actual_screen_size.y/screen_size.x;
        final_matrix = get_translation_matrix(Vec3(0.f, -screen_portion_of_ui_top+screen_portion_of_ui_bottom, 0.f)) * get_scale_matrix(other, scale_factor, 1.f) * get_translation_matrix(Vec3(screen_translate.x, screen_translate.y, 0.f));
        Vec2 ssil = Vec2(1.f/other, shown_level_size.y/2.f);
        screen_size_in_level_bl = -ssil - screen_translate;
        screen_size_in_level_tr =  ssil - screen_translate;
    }
    
    
    Mat4 ui_matrix = get_translation_matrix(Vec3(-1.f, -1.f, 0.f)) * get_scale_matrix(2.f, 2.f/actual_screen_ratio, 1.f);
    
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
                image_data[k++] = level_color.r+r;
                image_data[k++] = level_color.g+r;
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
    
    glUseProgram(ptt_program.id);
    glUniformMatrix4fv(ptt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    float t = 1.f;//-0.4f*MAX(MIN(rendered_player.cancel_next_level_in, 1.f), 0);
    glUniform3f(ptt_program.color_multiplier, 1.f, t, t);
    
    if(noise_buffer.count > 0){
        glUniform1i(ptt_program.sampler0, 1);
        glUniform1i(ptt_program.sampler1, 3);
        glUniform1f(ptt_program.t, rendered_player.time);
        glBindVertexArray(noise_vao);
        glDrawArrays(GL_TRIANGLES, 0, noise_buffer.count);
        check_openGL_error();
    }
    
    glUseProgram(pt_program.id);
    glUniformMatrix4fv(pt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    glUniform3f(pt_program.color_multiplier, 1.f, t, t);
    
    if(player_buffer.count > 0){
        glUniform1i(pt_program.sampler, 0);
        glBindVertexArray(player_vao);
        glDrawArrays(GL_TRIANGLES, 0, player_buffer.count);
        check_openGL_error();
    }
    
    if(ui_textures_buffer.count > 0){
        glUniformMatrix4fv(pt_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(ui_textures_vao);
        glDrawArrays(GL_TRIANGLES, 0, ui_textures_buffer.count);
        check_openGL_error();
    }

    glUseProgram(pca_program.id);
    glUniformMatrix4fv(pca_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    
    if(level_buffer.count > 0){
        glBindVertexArray(level_vao);
        glDrawArrays(GL_TRIANGLES, 0, level_buffer.count);
        check_openGL_error();
    }
    if(level_changing_buffer.count > 0){
        glBindVertexArray(level_changing_vao);
        glDrawArrays(GL_TRIANGLES, 0, level_changing_buffer.count);
        check_openGL_error();
    }
    if(goal_buffer.count > 0){
        glBindVertexArray(goal_vao);
        glDrawArrays(GL_TRIANGLES, 0, goal_buffer.count);
        check_openGL_error();
    }
    
    if(particle_buffer.count > 0){
        glBindVertexArray(particle_vao);
        glDrawArrays(GL_TRIANGLES, 0, particle_buffer.count);
        check_openGL_error();
    }
    
    if(loading_buffer.count > 0){
        glBindVertexArray(loading_vao);
        glUniformMatrix4fv(pca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glDrawArrays(GL_TRIANGLES, 0, loading_buffer.count);
        check_openGL_error();
    }
    
    if(level_moving_buffer.count > 0){
        Mat4 matrix = final_matrix * get_translation_matrix(Vec3(0.f, rendered_player.level_moving_y, 0.f));
        glUniformMatrix4fv(pca_program.matrix, 1, GL_FALSE, &matrix.values[0][0]);
        glBindVertexArray(level_moving_vao);
        glDrawArrays(GL_TRIANGLES, 0, level_moving_buffer.count);
        check_openGL_error();
    }
    
    {
        start_temp_alloc();
        u32 vert_num = 0;
        
        String loading_level_string;
        loading_level_string.allocator = &temporary_storage.allocator;
        RgbaColor loading_level_color;
        float loading_shown_level_size_mult; // @Todo: if this is always 0.5f, don't make it a variable
        if(rendered_player.completed_level){
            loading_level_string = "Loading level...";
            loading_level_color = {255, 255, 255, 255};
            loading_shown_level_size_mult = 0.5f;
        }else if(rendered_player.cancel_next_level_in > 0.f){
            loading_level_string = "Cancelled";
            loading_level_color = {255, 50, 50, (u8)(MIN(rendered_player.cancel_next_level_in, 1.f)*255)};
            loading_shown_level_size_mult = 0.5f;
        }else{
            loading_level_string = "";
            loading_level_color = {255, 255, 255, 0};
            loading_shown_level_size_mult = 0.5f;
        }
        vert_num += text_vert_num(loading_level_string);
        
        float time = player.shown_time;
        
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
        if(player.lives >= 0)
            sprintf(lives_sc, "   x%i", player.lives);
        else
            sprintf(lives_sc, "   x(%i)", player.lives);
        lives_string = lives_sc;
        vert_num += text_vert_num(lives_string);
        
        
        Vertex_PTCa *o_verts = (Vertex_PTCa *)temp_alloc(vert_num*sizeof(Vertex_PTCa));
        Vertex_PTCa *verts = o_verts;
        
        float spb = screen_portion_of_ui_bottom * actual_screen_ratio;
        float spt = screen_portion_of_ui_top    * actual_screen_ratio;
        
        verts += render_text(0.5f*spb, 0.5f*spb, -0.9f, FONT_QUALITY_64, loading_level_string, verts, loading_shown_level_size_mult*spb, NULL, NULL, loading_level_color, TEXT_ALIGN_BOTTOM);
        float time_width;
        Vertex_PTCa *verts_time = verts + render_text_monospace(1.f, actual_screen_ratio+0.2f*spt, -0.9f, FONT_QUALITY_64, time_string, verts, spt, &time_width, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP);
        while(verts < verts_time){
            verts->p.x -= time_width;
            verts++;
        }
        verts += render_text_monospace(0.f, actual_screen_ratio+0.2f*spt, -0.9f, FONT_QUALITY_64, lives_string, verts, spt, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP);
    
        text_buffer.count = vert_num;
        set_buffer_data_static(text_buffer.buffer, o_verts, text_buffer.count);
        
        end_temp_alloc();
    }
    
    if(text_buffer.count > 0){
        glUseProgram(ptca_program.id);
        glUniformMatrix4fv(ptca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glUniform1i(ptca_program.sampler, 2);
        
        glBindVertexArray(text_vao);
        glDrawArrays(GL_TRIANGLES, 0, text_buffer.count);
        check_openGL_error();
    }
    
#if OS == OS_IOS
    {
        float pun = 0.2*actual_screen_ratio;
        float hpun = 0.5f*pun;
        float yun = 0.2*pun;
        const float _factor = 0.5f*(1.f-(0.6f*SQRT_3_2));
        float xun = _factor*pun;
        float margin = 0.02*actual_screen_ratio;
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
        buttons_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(buttons_buffer.buffer, o_vertices, buttons_buffer.count);
        
        glUseProgram(pca_program.id);
        glUniformMatrix4fv(pca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(buttons_vao);
        glDrawArrays(GL_TRIANGLES, 0, buttons_buffer.count);
        check_openGL_error();
    }
#endif
}

void change_window_size(){
    actual_screen_size.x = window_size.x;
    actual_screen_size.y = window_size.y;
    screen_size.x = actual_screen_size.x;
    screen_size.y = (1.f - screen_portion_of_ui_top - screen_portion_of_ui_bottom)*actual_screen_size.y;
    actual_screen_ratio = actual_screen_size.y / actual_screen_size.x;
    if(window_size.x >= window_size.y){
        screen_ratio.x = (float) window_size.y / window_size.x;
        screen_ratio.y = 1.f;
    }else{
        screen_ratio.x = 1.f;
        screen_ratio.y = (float) window_size.x / window_size.x;
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
        float xmargin = 0.9f*screen_portion_of_ui_top*actual_screen_ratio;
        float ymargin = (1.f-0.9f*screen_portion_of_ui_top)*actual_screen_ratio;
        float pun = 0.8*screen_portion_of_ui_top*actual_screen_ratio;
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
        ui_textures_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(ui_textures_buffer.buffer, o_vertices, ArrayCount(o_vertices));
    }
    {
        RgbaColor c = {155, 0, 0, 255};
        float y0 = screen_portion_of_ui_bottom * actual_screen_ratio;
        float y1 = screen_portion_of_ui_top    * actual_screen_ratio;
        Vertex_PCa o_vertices[] = {
            {Vec3(0.f, 0.f, -0.8f), c},
            {Vec3(1.f, 0.f, -0.8f), c},
            {Vec3(0.f, y0,   -0.8f), c},
            {Vec3(0.f, y0,   -0.8f), c},
            {Vec3(1.f, 0.f, -0.8f), c},
            {Vec3(1.f, y0,   -0.8f), c},
            
            {Vec3(0.f, actual_screen_ratio,    -0.8f), c},
            {Vec3(1.f, actual_screen_ratio,    -0.8f), c},
            {Vec3(0.f, actual_screen_ratio-y1, -0.8f), c},
            {Vec3(0.f, actual_screen_ratio-y1, -0.8f), c},
            {Vec3(1.f, actual_screen_ratio,    -0.8f), c},
            {Vec3(1.f, actual_screen_ratio-y1, -0.8f), c},
        };
        tmp_buffer.count = ArrayCount(o_vertices);
        set_buffer_data_dynamic(tmp_buffer.buffer, o_vertices, ArrayCount(o_vertices));
    }
}
