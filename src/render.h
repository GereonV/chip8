#ifndef CHIP8_RENDER_H
#define CHIP8_RENDER_H

void clear_screen();
bool draw_sprite(unsigned char const * sprite_data, unsigned char sprite_height, unsigned char x, unsigned char y);

#endif
