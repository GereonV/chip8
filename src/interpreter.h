#ifndef CHIP8_INTERPRETER_H
#define CHIP8_INTERPRETER_H

extern unsigned char delay_timer, sound_timer; // atomic

void load_program(char const * filename);
void fetch_decode_execute();

#endif
