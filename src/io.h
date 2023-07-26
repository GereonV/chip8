#ifndef CHIP8_IO_H
#define CHIP8_IO_H

void init_io();
bool is_key_pressed(unsigned char key);
unsigned char next_keypress();

#endif
