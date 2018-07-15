#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "timer.h"
#include "ui.h"

struct tick_handler_ctx {
	WINDOW *window;
};

void
bye(int status)
{
	ui_stop();
	exit(status);
}

void
tick_handler_callback(struct timespec *remaining, void *ctx, int error)
{
	struct tick_handler_ctx *context = ctx;
	int secs;
	/* HH:MM\0 */
	char buf[6];

	if (0 != error) {
		if (0 > sprintf(buf, "??:??")) bye(EXIT_FAILURE);
	} else {
		secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
		if (0 > sprintf(buf, "%02d:%02d", secs / 60, secs % 60))
			bye(EXIT_FAILURE);
	}

	if (NULL != context && NULL != context->window)
		if (ERR == ui_set_centered(context->window, buf))
			bye(EXIT_FAILURE);
}

struct timer_thread_ctx {
	long secs;
	tick_handler tick_handler;
	struct tick_handler_ctx *tick_handler_context;
};

void *
thread_timer_start(void *ctx)
{
	struct timer_thread_ctx *context = ctx;
	timer_start(context->secs, context->tick_handler, context->tick_handler_context);
	pthread_exit(NULL);
}

int
main(int argc, char *argv[])
{
	char *endptr;
	WINDOW *win = NULL;
	struct tick_handler_ctx handler_ctx;
	struct timer_thread_ctx thread_ctx;
	pthread_attr_t timer_thread_attr;
	pthread_t timer_thread;

	if (argc != 2) {
		fprintf(stderr, "%s <duration in seconds>\n", argv[0]);
		return(EXIT_FAILURE);
	}

	thread_ctx.secs = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s is invalid (did you mean %ld?)\n", argv[1], thread_ctx.secs);
		return(EXIT_FAILURE);
	}
	if (thread_ctx.secs <= 0) {
		fprintf(stderr, "duration (%ld) must be greater than zero.\n", thread_ctx.secs);
		return(EXIT_FAILURE);
	}

	win = ui_start();
	if (NULL == win) {
		ui_stop();
		fprintf(stderr, "could not initialize window\n");
		return(EXIT_FAILURE);
	}

	handler_ctx.window = win;
	thread_ctx.tick_handler_context = &handler_ctx;
	thread_ctx.tick_handler = tick_handler_callback;

	printf("about to start\n");
	pthread_attr_init(&timer_thread_attr);
	pthread_create(&timer_thread, &timer_thread_attr,
			thread_timer_start, &thread_ctx);
	pthread_join(timer_thread, NULL);
	if (0 != ui_stop()) {
		fprintf(stderr, "failure stopping window\n");
		return(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
