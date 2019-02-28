#ifndef render_hpp
#define render_hpp

#include "basecode/basecode.hpp"
#include "basecode/opengl.hpp"

extern Vec2 level_size;
const float screen_portion_of_ui_bottom = 0.1f;
const float screen_portion_of_ui_top    = 0.07f;

struct BufferAndCount {
    GLuint buffer;
    u32 count;
};

struct GLObjects {
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
    BufferAndCount level_changing_buffers[2];
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
    GLuint level_changing_vaos[2];
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
    Vec2 screen_translate;
    Vec2 shown_level_size;
    
    Vec2 screen_size;
    Vec2 actual_screen_size;
    float actual_screen_ratio;
    
};

void init_openGL(GameState *game_state);
void draw_scene(GameState *game_state, bool should_redraw_level, bool should_redraw_state);
void change_window_size(GameState *game_state);

#endif /* render_hpp */