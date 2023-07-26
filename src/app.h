#ifndef CHIP8_APP_H
#define CHIP8_APP_H

#include <stdio.h>

#define ERROR(msg, code) \
	do { \
		fprintf(stderr, "[ERROR] %s\n", msg); \
		exit(code); \
	} while(false);

#endif
