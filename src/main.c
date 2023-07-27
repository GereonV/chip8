#include <stdatomic.h>
#include <stdlib.h>
#include <threads.h>
#include <GLFW/glfw3.h>
#include "app.h"
#include "interpreter.h"
#include "io.h"
#include "render.h"

static void error_callback_function(int code, char const * description) {
	ERROR(description, code - 0x10000);
}

static int interpreter_thread_function(void *) {
	struct timespec x = { .tv_nsec = 5'000'000, };
	while(true) {
		fetch_decode_execute();
		thrd_sleep(&x, nullptr);
	}
	return 0;
}

static void tick_timer(unsigned char * timer) {
	if(atomic_load_explicit(timer, memory_order_relaxed))
		atomic_fetch_sub_explicit(timer, 1, memory_order_relaxed);
}

static int timers_thread_function(void *) {
	struct timespec x = { .tv_nsec = 16'600'000, };
	while(true) {
		tick_timer(&delay_timer);
		tick_timer(&sound_timer);
		thrd_sleep(&x, nullptr);
	}
	return 0;
}

static void spawn_thread(thrd_start_t func) {
	thrd_t thr;
	if(!thrd_create(&thr, func, nullptr) == thrd_success)
		ERROR("creation of thread failed", 1);
}

int main(int argc, char *argv[argc]) {
	if(argc != 2)
		ERROR("invalid usage: <exe> <filename>", 50);
	glfwSetErrorCallback(error_callback_function);
	glfwInit();
	atexit(glfwTerminate);
	// TODO window creation hints
	auto window = glfwCreateWindow(1280, 640, "CHIP-8", nullptr, nullptr);
	glfwMakeContextCurrent(window);
	init_render();
	init_io();
	load_program(argv[1]);
	spawn_thread(interpreter_thread_function);
	spawn_thread(timers_thread_function);
	while(!glfwWindowShouldClose(window)) {
		render();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
