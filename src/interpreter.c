#include "interpreter.h"
#include <stdatomic.h>
#include <stdlib.h>
#include "io.h"
#include "render.h"

static unsigned char registers[16];
static unsigned short address_register;
static unsigned char memory[4096] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xA0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};
unsigned char delay_timer, sound_timer; // atomic
static unsigned short program_counter, stack[12], * stack_pointer = stack;

void fetch_decode_execute() {
	auto high_byte = memory[program_counter++];
	auto  low_byte = memory[program_counter++];
	auto x = registers + (high_byte & 0b1111);
	switch(high_byte >> 4) { // highest nibble
	case 0:
		// 0x0NNN is deprecated
		switch(low_byte) {
		case 0xE0:
			clear_screen();
			break;
		case 0xEE:
			program_counter = *--stack_pointer;
			break;
		}
		break;
	case 2:
		*stack_pointer++ = program_counter;
		[[fallthrough]];
	case 1:
		program_counter = high_byte << 8 | low_byte;
		break;
	case 3:
		if(*x == low_byte)
			program_counter += 2;
		break;
	case 4:
		if(*x != low_byte)
			program_counter += 2;
		break;
	case 5:
		if(*x == registers[low_byte >> 4])
			program_counter += 2;
		break;
	case 9:
		if(*x != registers[low_byte >> 4])
			program_counter += 2;
		break;
	case 6:
		*x = low_byte;
		break;
	case 7:
		*x += low_byte;
		break;
	case 8:
		auto y = registers[low_byte >> 4];
		switch(low_byte & 0b1111) {
		case 0:
			*x = y;
			break;
		case 1:
			*x |= y;
			break;
		case 2:
			*x &= y;
			break;
		case 3:
			*x ^= y;
			break;
		case 4:
			registers[15] = (*x += y) < y;
			break;
		case 5:
			registers[15] = *x >= y;
			*x -= y;
			break;
		case 6:
			registers[15] = y & 1;
			*x = y >> 1;
			break;
		case 7:
			registers[15] = y >= *x;
			*x = y - *x;;
			break;
		case 0xE:
			registers[15] = y & 0b10000000;
			*x = y << 1;
			break;
		}
		break;
	case 0xA:
		address_register = high_byte << 8 | low_byte;
		break;
	case 0xB:
		program_counter = (unsigned short) (high_byte << 8 | low_byte) + registers[0];
		break;
	case 0xC:
		*x = (unsigned char) rand() & low_byte;
		break;
	case 0xD:
		registers[15] = draw_sprite(memory + address_register, low_byte & 0b1111, *x, registers[low_byte >> 4]);
		break;
	case 0xE:
		switch(low_byte) {
		case 0x9E:
			if(is_key_pressed(*x))
				program_counter += 2;
			break;
		case 0xA1:
			if(!is_key_pressed(*x))
				program_counter += 2;
			break;
		}
		break;
	case 0xF:
		switch(low_byte) {
		case 7:
			*x = atomic_load_explicit(&delay_timer, memory_order_relaxed);
			break;
		case 0xA:
			*x = next_keypress();
			break;
		case 0x15:
			atomic_store_explicit(&delay_timer, *x, memory_order_relaxed);
			break;
		case 0x18:
			atomic_store_explicit(&sound_timer, *x, memory_order_relaxed);
			break;
		case 0x1E:
			address_register += *x;
			break;
		case 0x29:
			address_register = 5 * *x;
			break;
		case 0x33:
			memory[address_register + 0] = *x / 100;
			memory[address_register + 1] = (*x / 10) % 10;
			memory[address_register + 2] = *x % 10;
			break;
		case 0x55:
			for(auto r = registers; r <= x; ++r)
				memory[address_register++] = *r;
			break;
		case 0x65:
			for(auto r = registers; r <= x; ++r)
				*r = memory[address_register++];
			break;
		}
		break;
	}
}
