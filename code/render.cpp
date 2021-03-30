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
    // Enable the things which are necessary everywhere
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#if !GGTP_OPENGL_ES
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.02f);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
#endif
    
    glEnable(GL_SCISSOR_TEST);
    
    
    // Initialize all the shaders
    GLObjects *gl = &game_state->gl_objects;
    const char pca_shader_v[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pca_shader_v.glsl"
#else
#include "shaders/pca_shader_v_es.glsl"
#endif
    ;
    const char pca_shader_f[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pca_shader_f.glsl"
#else
#include "shaders/pca_shader_f_es.glsl"
#endif
    ;
    gl->pca_program.id = ggtgl_load_shaders_by_text(pca_shader_v, pca_shader_f);
    gl->pca_program.position = glGetAttribLocation (gl->pca_program.id, "a_position");
    gl->pca_program.color    = glGetAttribLocation (gl->pca_program.id, "a_color");
    gl->pca_program.matrix   = glGetUniformLocation(gl->pca_program.id, "u_matrix");
    
    const char pt_shader_v[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pt_shader_v.glsl"
#else
#include "shaders/pt_shader_v_es.glsl"
#endif
    ;
    const char pt_shader_f[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pt_shader_f.glsl"
#else
#include "shaders/pt_shader_f_es.glsl"
#endif
    ;
    gl->pt_program.id = ggtgl_load_shaders_by_text(pt_shader_v, pt_shader_f);
    gl->pt_program.position   = glGetAttribLocation (gl->pt_program.id, "a_position");
    gl->pt_program.tex_coords = glGetAttribLocation (gl->pt_program.id, "a_tex_coords");
    gl->pt_program.matrix     = glGetUniformLocation(gl->pt_program.id, "u_matrix");
    gl->pt_program.sampler    = glGetUniformLocation(gl->pt_program.id, "u_sampler");
    
    const char pta_shader_v[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pta_shader_v.glsl"
#else
#include "shaders/pta_shader_v_es.glsl"
#endif
    ;
    const char pta_shader_f[] =
    #if !GGTP_OPENGL_ES
#include "shaders/pta_shader_f.glsl"
#else
#include "shaders/pta_shader_f_es.glsl"
#endif
    ;
    gl->pta_program.id = ggtgl_load_shaders_by_text(pta_shader_v, pta_shader_f);
    gl->pta_program.position   = glGetAttribLocation (gl->pta_program.id, "a_position");
    gl->pta_program.tex_coords = glGetAttribLocation (gl->pta_program.id, "a_tex_coords");
    gl->pta_program.matrix     = glGetUniformLocation(gl->pta_program.id, "u_matrix");
    gl->pta_program.sampler    = glGetUniformLocation(gl->pta_program.id, "u_sampler");
    
    const char ptca_shader_v[] =
    #if !GGTP_OPENGL_ES
#include "shaders/ptca_shader_v.glsl"
#else
#include "shaders/ptca_shader_v_es.glsl"
#endif
    ;
    const char ptca_shader_f[] =
    #if !GGTP_OPENGL_ES
#include "shaders/ptca_shader_f.glsl"
#else
#include "shaders/ptca_shader_f_es.glsl"
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
    
    
    // Initialize the textures
    glActiveTexture(GL_TEXTURE0);
    char path[MAX_PATH_LENGTH];
    ggtp_program_file_path("Textures/textures.png", path);
    ggtgl_load_texture(&gl->main_texture, path);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    ggtgl_check_error();
    
    glActiveTexture(GL_TEXTURE1);
    ggtp_program_file_path("Textures/planet.png", path);
    ggtgl_load_texture(&gl->planet_texture, path);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if OS == OS_WASM
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#endif
    ggtgl_check_error();
    
    // Set up the framebuffer and renderbuffer (to control render quality)
    glGenFramebuffers(1, &gl->resolved_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->resolved_fbo);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gl->framebuffer_texture, 0);
    ggtgl_check_error();
    
    GLuint rbo0;
    glGenRenderbuffers(1, &rbo0);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo0); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, (int)MAX_RESOLUTION_ROUNDED_X, (int)MAX_RESOLUTION_ROUNDED_Y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo0);
    ggtgl_check_error();
    
    
    GLenum drawbuffers[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawbuffers);
    ggtgl_check_error();
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Error: Framebuffer 1 is not complete (0x%x).\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    
    
    glGenFramebuffers(1, &gl->multisampled_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->multisampled_fbo);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, tex4, 0);
    ggtgl_check_error();
    
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo); 
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, (int)MAX_RESOLUTION_ROUNDED_X, (int)MAX_RESOLUTION_ROUNDED_Y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    ggtgl_check_error();
    
    glGenRenderbuffers(1, &gl->depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, gl->depth_rbo); 
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, MAX_RESOLUTION_ROUNDED_X, MAX_RESOLUTION_ROUNDED_Y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gl->depth_rbo);
    //glBindRenderbuffer(GL_RENDERBUFFER, 0);
    ggtgl_check_error();
    
    glDrawBuffers(1, drawbuffers);
    ggtgl_check_error();
    
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("Error: Framebuffer 2 is not complete (0x%x).\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    ggtgl_check_error();
    
    change_window_size(game_state, (Vec2)window_size);
    
    ggtgl_check_error();
    
    
    // Blit the framebuffers once, since for some reason the first time it is done it is very slow
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gl->resolved_fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->multisampled_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->resolved_fbo);
        glBlitFramebuffer(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, 0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        ggtgl_check_error();
        
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int dx = (int)gl->actual_screen_size.x;
        int dy = (int)gl->actual_screen_size.y;
        glViewport(0, 0, dx, dy);
        glScissor(0, 0, dx, dy);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->resolved_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, 0, 0, dx, dy, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        
        ggtgl_check_error();
    }
    
}

#if MEASURE_RENDER_TIME
#include <time.h>
const int time_measures_size = 100;
clock_t time_measures[time_measures_size];
#define start_time() clock_t __t0 = clock()
#define print_time(num) { glFinish(); clock_t t1 = clock(); time_measures[num] += t1-__t0; __t0 = t1; }
void print_render_time(){
    for(int i=0; i<time_measures_size; i++){
        clock_t val = time_measures[i];
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

#if OS == OS_WASM || !defined(DEBUG_BUILD)
#define DONT_CHECK_ERRORS 1
#else
#define DONT_CHECK_ERRORS 0
#endif

#if DONT_CHECK_ERRORS
#undef ggtgl_check_error
#define ggtgl_check_error()
#endif


void load_ui_text_into_buffer(GameState *game_state, BufferAndCount *buffer){
    GLObjects *gl = &game_state->gl_objects;
    
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
    Vertex_PTCa *verts_time = verts + render_text(1.f, gl->actual_screen_ratio+0.2f*spt, -0.8f, FONT_QUALITY_64, time_sc, (mt_Vertex_PTCa *)verts, spt, &time_width, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP | TEXT_MONOSPACE);
    while(verts < verts_time){
        verts->p.x -= time_width;
        verts++;
    }
    verts += render_text(0.f, gl->actual_screen_ratio+0.2f*spt, -0.8f, FONT_QUALITY_64, lives_sc, (mt_Vertex_PTCa *)verts, spt, NULL, NULL, {255, 255, 255, 255}, TEXT_ALIGN_TOP | TEXT_MONOSPACE);
    
    buffer->count = vert_num;
    ggtgl_set_buffer_data(buffer->buffer, o_verts, buffer->count, GL_DYNAMIC_DRAW);
    
    end_temp_alloc();
}

#define DRAW_FPS_GRAPHIC 0

#if DRAW_FPS_GRAPHIC

int measure_time_comp(const void *a, const void *b){
    float d = *(float *)a - *(float *)b;
    return d > 0.f ? 1 : -1;
}

void measure_time(){
#define RECORDED_FRAMES 512
    static float frame_times[RECORDED_FRAMES];
    static int next_frame = 0;
    static clock_t last_time = clock();
    
    clock_t this_time = clock();
    float this_frame_time = (float)(this_time - last_time)/CLOCKS_PER_SEC;
    last_time = this_time;
    
    frame_times[next_frame] = this_frame_time;
    next_frame = (next_frame+1) % RECORDED_FRAMES;
    
    float ordered_times[RECORDED_FRAMES];
    memcpy(ordered_times, frame_times, sizeof(frame_times));
    qsort(ordered_times, RECORDED_FRAMES, sizeof(float), measure_time_comp);
    
    u8 rgb[2*RECORDED_FRAMES][2*RECORDED_FRAMES][3];
    
    float total_time = 0.f;
    for(int f=0; f<RECORDED_FRAMES; f++){
        total_time += ordered_times[f];
    }
    
#if 0
    
    int average = (int)roundf(RECORDED_FRAMES * (RECORDED_FRAMES/total_time) / 60.f);
    
    
    for(int f=0; f<RECORDED_FRAMES; f++){
        int ff = f;
        int h = (int)roundf(RECORDED_FRAMES * (1.f/ordered_times[f]) / 60.f);
        int y;
        for(y=0; y<=h && y<2*RECORDED_FRAMES; y++){
            rgb[y][ff][0] = 255;
            rgb[y][ff][1] = 0;
            rgb[y][ff][2] = 0;
        }
        for(; y<2*RECORDED_FRAMES; y++){
            if(y == RECORDED_FRAMES){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 0;
            }else if(y == average){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 0;
                rgb[y][ff][2] = 255;
            }else{
                rgb[y][ff][0] = 255;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 255;
            }
        }
    }
    for(int f=0; f<RECORDED_FRAMES; f++){
        int ff = f+RECORDED_FRAMES;
        int h = (int)roundf(RECORDED_FRAMES * (1.f/frame_times[f]) / 60.f);
        int y;
        for(y=0; y<=h && y<2*RECORDED_FRAMES; y++){
            rgb[y][ff][0] = 255;
            rgb[y][ff][1] = 0;
            rgb[y][ff][2] = 0;
        }
        for(; y<2*RECORDED_FRAMES; y++){
            if(y == RECORDED_FRAMES){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 0;
            }else if(y == average){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 0;
                rgb[y][ff][2] = 255;
            }else{
                rgb[y][ff][0] = 255;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 255;
            }
        }
    }
#else
    int average = (int)roundf(2.f*RECORDED_FRAMES * 1000.f * (total_time/RECORDED_FRAMES) / 256.f);
    
    
    for(int f=0; f<RECORDED_FRAMES; f++){
        int ff = f;
        int h = (int)roundf(2.f*RECORDED_FRAMES * 1000.f*ordered_times[f] / 256.f);
        int y;
        for(y=0; y<=h && y<2*RECORDED_FRAMES; y++){
            rgb[y][ff][0] = 255;
            rgb[y][ff][1] = 0;
            rgb[y][ff][2] = 0;
        }
        for(; y<2*RECORDED_FRAMES; y++){
            if(y == 2*RECORDED_FRAMES/256*17){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 0;
            }else if(y == average){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 0;
                rgb[y][ff][2] = 255;
            }else{
                rgb[y][ff][0] = 255;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 255;
            }
        }
    }
    for(int f=0; f<RECORDED_FRAMES; f++){
        int ff = f+RECORDED_FRAMES;
        int h = (int)roundf(2.f*RECORDED_FRAMES * 1000.f*frame_times[f] / 256.f);
        int y;
        for(y=0; y<=h && y<2*RECORDED_FRAMES; y++){
            rgb[y][ff][0] = 255;
            rgb[y][ff][1] = 0;
            rgb[y][ff][2] = 0;
        }
        for(; y<2*RECORDED_FRAMES; y++){
            if(y == 2*RECORDED_FRAMES/256*17){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 0;
            }else if(y == average){
                rgb[y][ff][0] = 0;
                rgb[y][ff][1] = 0;
                rgb[y][ff][2] = 255;
            }else{
                rgb[y][ff][0] = 255;
                rgb[y][ff][1] = 255;
                rgb[y][ff][2] = 255;
            }
        }
    }
#endif
    
    
    glActiveTexture(GL_TEXTURE1);
    static GLuint texture = 0;
    if(texture == 0){
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2*RECORDED_FRAMES, 2*RECORDED_FRAMES, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb);
}

#endif

void draw_scene(GameState *game_state, float time_step, bool should_redraw_level, bool draw_ui_and_player){
    start_time();
    
    GLObjects *gl = &game_state->gl_objects;
    PlayerSnapshot *rendered_player = &game_state->rendered_player;
    
    find_snapshot_to_render(game_state);
    load_player_into_buffer(game_state, time_step, &gl->player_buffer); // This must be done first, because it sets rendered_player
    load_particles_into_buffer(game_state, &gl->particle_buffer);
    load_ui_text_into_buffer(game_state, &gl->text_buffer);
    if(should_redraw_level){
        memcpy(&game_state->lagged_level, &game_state->next_lagged_level, sizeof(LaggedLevel));
        
        
        load_level_into_buffer(game_state, &gl->level_buffer, &gl->level_changing_buffers[0], &gl->level_changing_buffers[1]);
        
        gl->drawn_level_state = 0;
        load_planet_background(&game_state->level, &gl->planet_buffer);
    }
    load_dynamic_level_parts_into_buffer(game_state, &gl->goal_buffer, time_step);
    
    float scale_to_match_x = 2.f/gl->shown_level_size.x;
    float scale_to_match_y = 2.f/gl->shown_level_size.y * gl->actual_screen_ratio;
    float scale_to_match_y_with_ui = 2.f/(gl->shown_level_size.y-1.f) * (gl->screen_size.y / gl->actual_screen_size.x);
    float min_scale = fminf(scale_to_match_x, fminf(scale_to_match_y, scale_to_match_y_with_ui));
    
    Mat4 final_matrix = get_scale_matrix(min_scale, min_scale / gl->actual_screen_ratio, 1.f) * get_translation_matrix(Vec3(gl->screen_translate.x, gl->screen_translate.y, 0.f));
    
    if(min_scale == scale_to_match_y_with_ui)
        final_matrix = get_translation_matrix(Vec3(0.f, -0.5f*screen_portion_of_ui_top, 0.f)) * final_matrix;
    
    Mat4 ui_matrix = get_translation_matrix(Vec3(-1.f, -1.f, 0.f)) * get_scale_matrix(2.f, 2.f/gl->actual_screen_ratio, 1.f);
#if DRAW_FPS_GRAPHIC
    measure_time();
#endif
    
    print_time(0);
    
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gl->multisampled_fbo);
        
        glViewport(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y);
        glScissor(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y);
    }
    
    glClearColor(basic_color.r/255.f, basic_color.g/255.f, basic_color.b/255.f, 1.f);
    //glClearColor(0.f, 0.f, 0.f, 1.f);
    //glClearColor(45.f/255.f, 45.f/255.f, 45.f/255.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    
    glUseProgram(gl->pca_program.id);
    glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    
    if(gl->level_buffer.count > 0){
        glBindVertexArray(gl->level_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_buffer.count);
        ggtgl_check_error();
        //printf("%i\n", gl->level_buffer.count);
    }
    
    if(gl->level_changing_buffers[gl->drawn_level_state].count > 0){
        glBindVertexArray(gl->level_changing_vaos[gl->drawn_level_state]);
        glDrawArrays(GL_TRIANGLES, 0, gl->level_changing_buffers[gl->drawn_level_state].count);
        ggtgl_check_error();
    }
    
    
    print_time(4);
    
    
    glUseProgram(gl->pt_program.id);
    glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &final_matrix.values[0][0]);
    
    if(gl->planet_buffer.count > 0){
        glUniform1i(gl->pt_program.sampler, 1);
        glBindVertexArray(gl->planet_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->planet_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(1);
    
    if(draw_ui_and_player && gl->player_buffer.count > 0){
        bool must_scissor = (!game_state->is_in_real_game && game_state->game_mode == GAME_MODE_PLAY) || game_state->lagged_level.num == 0;
        if(must_scissor){
            // (we do this so that when the player finishes a level in "level select"
            //  you can't see the player falling below the level)
            int size_x = (int)(0.5f * min_scale * gl->shown_level_size.x * gl->framebuffer_size.x);
            int size_y = (int)(0.5f * min_scale * gl->shown_level_size.y * gl->framebuffer_size.x);
            int x = (gl->framebuffer_size.x - size_x)/2;
            int y = (gl->framebuffer_size.y - size_y)/2;
            glScissor(x, y, size_x, size_y);
        }
        
        glUniform1i(gl->pt_program.sampler, 0);
        glBindVertexArray(gl->player_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->player_buffer.count);
        ggtgl_check_error();
        
        if(must_scissor){
            glScissor(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y);
        }
    }
    
    if(draw_ui_and_player && gl->ui_textures_buffer.count > 0){
        glUniformMatrix4fv(gl->pt_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glBindVertexArray(gl->ui_textures_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->ui_textures_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(3);
    
    if(gl->goal_buffer.count > 0){
        glUseProgram(gl->pca_program.id);
        glBindVertexArray(gl->goal_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->goal_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(5);
    
    if(gl->particle_buffer.count > 0){
        if(gl->goal_buffer.count == 0) glUseProgram(gl->pca_program.id);
        glBindVertexArray(gl->particle_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->particle_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(6);
    
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gl->resolved_fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->multisampled_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, gl->resolved_fbo);
        glBlitFramebuffer(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, 0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        ggtgl_check_error();
        
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        int x0 = gl->viewport_offset.x;
        int y0 = gl->viewport_offset.y;
        int dx = (int)gl->actual_screen_size.x;
        int dy = (int)gl->actual_screen_size.y;
        glViewport(x0, y0, dx, dy);
        glScissor(x0, y0, dx, dy);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gl->resolved_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, gl->framebuffer_size.x, gl->framebuffer_size.y, x0, y0, x0+dx, y0+dy, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        
        ggtgl_check_error();
    }
    
    if(draw_ui_and_player && gl->text_buffer.count > 0){
        glUseProgram(gl->ptca_program.id);
        glUniformMatrix4fv(gl->ptca_program.matrix, 1, GL_FALSE, &ui_matrix.values[0][0]);
        glUniform1i(gl->ptca_program.sampler, 2);
        
        glBindVertexArray(gl->text_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->text_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(7);
    
    if(game_state->menu_info.fade_alpha >= 0.f){
        glUseProgram(gl->pca_program.id);
        
        game_state->menu_info.fade_alpha += game_state->menu_info.fade_direction * fade_speed * time_step;
        
        
        float alpha = CLAMP(game_state->menu_info.fade_alpha, 0.f, 1.f);
        
        RgbaColor c = {game_state->menu_info.fade_color.r, game_state->menu_info.fade_color.g, game_state->menu_info.fade_color.b, (u8)(alpha*255)};
        
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
        
        glUniformMatrix4fv(gl->pca_program.matrix, 1, GL_FALSE, &id.values[0][0]);
        glBindVertexArray(gl->fade_vao);
        glDrawArrays(GL_TRIANGLES, 0, gl->fade_buffer.count);
        ggtgl_check_error();
    }
    
    print_time(8);
    
    
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
        gl->screen_ratio.x = (float) size.y / size.x;
        gl->screen_ratio.y = 1.f;
    }else{
        gl->screen_ratio.x = 1.f;
        gl->screen_ratio.y = (float) size.x / size.x;
    }
    
    // Check if we need to cap the quality, i.e. use the framebuffer
    if(size.x > MAX_RESOLUTION_X || size.y > MAX_RESOLUTION_Y){
        if(gl->actual_screen_ratio > MAX_RESOLUTION_Y / MAX_RESOLUTION_X){
            gl->framebuffer_size.x = (int)(MAX_RESOLUTION_Y / gl->actual_screen_ratio);
            gl->framebuffer_size.y = (int)(MAX_RESOLUTION_Y);
        }else{
            gl->framebuffer_size.x = (int)(MAX_RESOLUTION_X);
            gl->framebuffer_size.y = (int)(MAX_RESOLUTION_X * gl->actual_screen_ratio);
        }
    }else{
        gl->framebuffer_size.x = (int)gl->actual_screen_size.x;
        gl->framebuffer_size.y = (int)gl->actual_screen_size.y;
    }
    
    if(game_state->game_mode != GAME_MODE_PLAY){
        glViewport(0, 0, (int)size.x, (int)size.y);
        glScissor( 0, 0, (int)size.x, (int)size.y);
    }
    
    { // Change a UI buffer
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
        ggtgl_set_buffer_data(gl->ui_textures_buffer.buffer, o_vertices, ArraySize(o_vertices), GL_STATIC_DRAW);
    }
}