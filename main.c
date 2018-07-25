#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include "timer.h"
#include "ui.h"

static const struct timespec waittime = {
	.tv_sec = 0,
	.tv_nsec = 100000000,
};

enum status {
	GO,
	STOP
};

struct state {
	enum status status;
};

struct tick_handler_ctx {
	WINDOW *window;
	struct state *state;
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

	switch (context->state->status) {
		case GO:
			secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
			if (0 > sprintf(buf, "%02d:%02d", secs / 60, secs % 60))
				bye(EXIT_FAILURE);

			if (NULL != context && NULL != context->window)
				if (ERR == ui_set_centered(context->window, buf))
					bye(EXIT_FAILURE);
			break;
		case STOP:
			pthread_exit(NULL);
		default:
			pthread_exit(NULL);

	}

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
	struct state *state;
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
	struct ui_thread_ctx *context = ctx;
	pthread_cleanup_push(ui_thread_cleanup, NULL);
	context->window = ui_start();

	if (NULL == context->window) pthread_exit(NULL);
	while (context->state->status == GO && (c = wgetch(context->window)) != 'q')
		nanosleep(&waittime, NULL);
	context->state->status = STOP;
	pthread_cleanup_pop(true);
	pthread_exit(NULL);
}


int
main(int argc, char *argv[])
{
	char *endptr;
	int timeout = 0;
	struct state state;
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

	state.status = GO;

	ui_thread_ctx.window = NULL;
	ui_thread_ctx.state = &state;

	pthread_create(&ui_thread, NULL, ui_thread_start, &ui_thread_ctx);

	while (NULL == ui_thread_ctx.window && timeout++ < 10)
		nanosleep(&waittime, NULL);

	if (10 == timeout)
		return(EXIT_FAILURE);

	handler_ctx.window = ui_thread_ctx.window;
	handler_ctx.state = &state;
	timer_thread_ctx.tick_handler_context = &handler_ctx;
	timer_thread_ctx.tick_handler = tick_handler_callback;

	pthread_create(&timer_thread, NULL, thread_timer_start, &timer_thread_ctx);
	pthread_join(timer_thread, NULL);

	pthread_cancel(ui_thread);
	pthread_join(ui_thread, NULL);

	return EXIT_SUCCESS;
}
