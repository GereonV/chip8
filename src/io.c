#include "io.h"
#include <stdatomic.h>
#include <threads.h>
#include <GLFW/glfw3.h>

static bool blocked; // atomic
static unsigned char key_pressed;

static unsigned short keys;
static short keymap[] = {
	GLFW_KEY_X, // 0
	GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, // 1-3
	GLFW_KEY_Q, GLFW_KEY_W, GLFW_KEY_E, // 4-6
	GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, // 7-9
	GLFW_KEY_Y, GLFW_KEY_C, // 10-11 TODO maybe y -> z?
	GLFW_KEY_4, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_V // 12-15
};

static void key_callback(GLFWwindow *, int key, int, int action, int) {
	if(action == GLFW_REPEAT)
		return;
	unsigned char chip_key;
	// would be sooooo nice with avx512... :(
	for(unsigned char i = 0; i < 16; ++i) {
		if(keymap[i] == key) {
			chip_key = i;
			break;
		}
	}
	unsigned short mask = 1 << chip_key;
	if(action == GLFW_RELEASE) {
		keys &= ~mask;
		return;
	}
	keys |= mask;
	if(atomic_load_explicit(&blocked, memory_order_relaxed)) {
		key_pressed = chip_key;
		atomic_store_explicit(&blocked, false, memory_order_release);
	}
}

void init_io() {
	glfwSetKeyCallback(glfwGetCurrentContext(), key_callback);
}

bool is_key_pressed(unsigned char key) {
	return keys & (1 << key);
}

unsigned char next_keypress() {
	atomic_store_explicit(&blocked, true, memory_order_relaxed);
	thrd_yield();
	while(atomic_load_explicit(&blocked, memory_order_relaxed));
	atomic_thread_fence(memory_order_acquire);
	return key_pressed;
}
