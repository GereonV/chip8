#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include "interpreter.h"
#include "io.h"

static int interpreter_thread_function(void *) {
	// TODO setup memory
	while(true)
		fetch_decode_execute();
	return 0;
}

static void tick_timer(unsigned char * timer) {
	if(atomic_load_explicit(&timer, memory_order_relaxed))
		atomic_fetch_sub_explicit(&timer, 1, memory_order_relaxed);
}

static int timers_thread_function(void *) {
	struct timespec x = { .tv_nsec = 166'600'000, };
	while(true) {
		tick_timer(&delay_timer);
		tick_timer(&sound_timer);
		thrd_sleep(&x, nullptr);
	}
	return 0;
}

static void spawn_thread(thrd_start_t func) {
	thrd_t thr;
	if(thrd_create(&thr, func, nullptr) == thrd_success)
		return;
	fputs("[ERROR] creation of thread failed\n", stderr);
	exit(1);
}

int main() {
	puts("Hello World!");
	spawn_thread(interpreter_thread_function);
	spawn_thread(timers_thread_function);
	// TODO init glfw
	init_io(nullptr); // TODO
}
