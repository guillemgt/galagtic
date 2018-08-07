#ifndef sound_hpp
#define sound_hpp

void init_sound();
void cleanup_sound();
void play_sound(u8 sound_index);

const u8 SOUND_WALK_0    = 0;
const u8 SOUND_WALK_1    = 1;
const u8 SOUND_JUMP      = 2;
const u8 SOUND_DEATH     = 3;
const u8 SOUND_WIN       = 4;
const u8 SOUND_COLLISION = 5;

extern bool sound_on;

#endif /* sound_hpp */
