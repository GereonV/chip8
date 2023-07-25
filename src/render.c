#include "render.h"
#include <stdint.h>

static uint64_t screen __attribute__((vector_size(64 * 32 / 8)));

void clear_screen() {
	screen = (typeof(screen)) {};
}

bool draw_sprite(unsigned char const * sprite_data, unsigned char sprite_height, unsigned char x, unsigned char y) {
	x %= 64;
	y %= 32;
	auto end = y + sprite_height;
	if(end > 32)
		end = 32;
	auto result = false;
	for(; y < end; ++y, ++sprite_data) {
		auto data = (uint64_t) *sprite_data << (64 - 8) >> x;
		if(screen[y] & data)
			result = true;
		screen[y] ^= data;
	}
	return result;
}
