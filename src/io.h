#ifndef CHIP8_IO_H
#define CHIP8_IO_H

#include <GLFW/glfw3.h>

void init_io(GLFWwindow * window);
bool is_key_pressed(unsigned char key);
unsigned char next_keypress();

#endif
