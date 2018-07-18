#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
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
tick_handler_callback(struct timespec *remaining, void *ctx)
{
	struct tick_handler_ctx *context = ctx;
	int secs;
	/* HH:MM\0 */
	char buf[6];

	secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
	if (0 > sprintf(buf, "%02d:%02d", secs / 60, secs % 60))
		bye(EXIT_FAILURE);

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

struct ui_thread_ctx {
	WINDOW *window;
};

void
ui_thread_cleanup(void *arg)
{
	ui_stop();
}

void *
ui_thread_start(void *ctx)
{
	int c;
	struct timespec sleepspec = {
		.tv_sec = 0,
		.tv_nsec = 500000000,
	};
	struct ui_thread_ctx *context = ctx;
	pthread_cleanup_push(ui_thread_cleanup, NULL);
	context->window = ui_start();

	if (NULL == context->window) pthread_exit(NULL);
	while ((c = wgetch(context->window)) != 'q') {
		nanosleep(&sleepspec, NULL);
	}
	pthread_cleanup_pop(true);
	pthread_exit(NULL);
}


int
main(int argc, char *argv[])
{
	char *endptr;
	struct timespec sleepspec = {
		.tv_sec = 0,
		.tv_nsec = 500000000,
	};
	int timeout = 0;
	struct tick_handler_ctx handler_ctx;
	struct timer_thread_ctx timer_thread_ctx;
	struct ui_thread_ctx ui_thread_ctx;
	pthread_t timer_thread;
	pthread_t ui_thread;

	if (argc != 2) {
		fprintf(stderr, "%s <duration in seconds>\n", argv[0]);
		return(EXIT_FAILURE);
	}

	timer_thread_ctx.secs = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s is invalid (did you mean %ld?)\n", argv[1], timer_thread_ctx.secs);
		return(EXIT_FAILURE);
	}
	if (timer_thread_ctx.secs <= 0) {
		fprintf(stderr, "duration (%ld) must be greater than zero.\n", timer_thread_ctx.secs);
		return(EXIT_FAILURE);
	}

	ui_thread_ctx.window = NULL;

	pthread_create(&ui_thread, NULL, ui_thread_start, &ui_thread_ctx);

	while (NULL == ui_thread_ctx.window && timeout++ < 10)
		nanosleep(&sleepspec, NULL);

	if (10 == timeout)
		return(EXIT_FAILURE);

	handler_ctx.window = ui_thread_ctx.window;
	timer_thread_ctx.tick_handler_context = &handler_ctx;
	timer_thread_ctx.tick_handler = tick_handler_callback;

	pthread_create(&timer_thread, NULL, thread_timer_start, &timer_thread_ctx);
	pthread_join(timer_thread, NULL);

	pthread_cancel(ui_thread);
	pthread_join(ui_thread, NULL);

	return EXIT_SUCCESS;
}
