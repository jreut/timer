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
	WINDOW *window;
	long duration_seconds;
	tick_handler tick_handler;
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
	struct state *context = ctx;
	int secs;
	/* HH:MM\0 */
	char buf[6];

	switch (context->status) {
		case GO:
			secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
			if (0 > sprintf(buf, "%02d:%02d", secs / 60, secs % 60))
				bye(EXIT_FAILURE);

			if (NULL != context &&  NULL != context->window)
				if (ERR == ui_set_centered(context->window, buf))
					bye(EXIT_FAILURE);
			break;
		case STOP:
			pthread_exit(NULL);
		default:
			pthread_exit(NULL);

	}

}

void *
thread_timer_start(void *ctx)
{
	struct state *context = ctx;
	timer_start(context->duration_seconds, context->tick_handler, context);
	pthread_exit(NULL);
}

void
ui_thread_cleanup(void *arg)
{
	ui_stop();
}

void *
ui_thread_start(void *ctx)
{
	int c;
	struct state *context = ctx;
	pthread_cleanup_push(ui_thread_cleanup, NULL);
	context->window = ui_start();

	if (NULL == context->window) pthread_exit(NULL);
	while (context->status == GO && (c = wgetch(context->window)) != 'q')
		nanosleep(&waittime, NULL);
	context->status = STOP;
	pthread_cleanup_pop(true);
	pthread_exit(NULL);
}


int
main(int argc, char *argv[])
{
	char *endptr;
	int timeout;
	struct state state;
	pthread_t timer_thread;
	pthread_t ui_thread;

	if (argc != 2) {
		fprintf(stderr, "%s <duration in seconds>\n", argv[0]);
		return(EXIT_FAILURE);
	}

	state.duration_seconds = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s is invalid (did you mean %ld?)\n", argv[1], state.duration_seconds);
		return(EXIT_FAILURE);
	}
	if (state.duration_seconds <= 0) {
		fprintf(stderr, "duration (%ld) must be greater than zero.\n", state.duration_seconds);
		return(EXIT_FAILURE);
	}

	state.status = GO;
	state.window = NULL;
	state.tick_handler = tick_handler_callback;

	pthread_create(&ui_thread, NULL, ui_thread_start, &state);

	timeout = 0;
	while (NULL == state.window && timeout++ < 10)
		nanosleep(&waittime, NULL);

	if (10 == timeout)
		return(EXIT_FAILURE);

	pthread_create(&timer_thread, NULL, thread_timer_start, &state);
	pthread_join(timer_thread, NULL);

	pthread_cancel(ui_thread);
	pthread_join(ui_thread, NULL);

	return EXIT_SUCCESS;
}
