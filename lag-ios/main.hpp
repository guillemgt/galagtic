#ifndef main_hpp
#define main_hpp

#include <stdio.h>

#if defined __cplusplus
extern "C" {
#endif
void init_thing(int width, int height, int def_fb);
void draw_scene();
    void do_buffers();
    void set_distance(float dist);
#if defined __cplusplus
};
#endif

#endif /* main_hpp */
