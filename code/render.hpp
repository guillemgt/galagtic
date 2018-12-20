#ifndef render_hpp
#define render_hpp

#include "basecode/basecode.hpp"
#include "basecode/opengl.hpp"

extern Vec2 screen_scale;
extern Vec2 screen_translate;
extern Vec2 level_size;
extern Vec2 shown_level_size;
extern Vec2 actual_screen_size;
extern float actual_screen_ratio;
const float screen_portion_of_ui_bottom = 0.1f;
const float screen_portion_of_ui_top    = 0.07f;

struct BufferAndCount {
    GLuint buffer;
    u32 count;
};
extern BufferAndCount text_buffer;
extern GLuint text_vao;

void init_openGL();
void draw_scene();
void change_window_size();

#endif /* render_hpp */
