#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "ui.h"

struct ctx {
	WINDOW *window;
};

void
handler(struct timespec *remaining, void *ctx, int error)
{
	struct ctx *context = ctx;
	int secs;
	/* HH:MM\0 */
	char buf[6];

	if (0 != error) {
		if (0 > sprintf(buf, "??:??"))
			exit(EXIT_FAILURE);
	} else {
		secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
		if (0 > sprintf(buf, "%02d:%02d", secs / 60, secs % 60))
			exit(EXIT_FAILURE);
	}

	if (NULL != context && NULL != context->window)
		if (ERR == ui_set_centered(context->window, buf))
			exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	long secs;
	char *endptr;
	WINDOW *win;
	struct ctx ctx;

	if (argc != 2) {
		fprintf(stderr, "%s <duration in seconds>\n", argv[0]);
		return(EXIT_FAILURE);
	}

	secs = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		fprintf(stderr, "%s is invalid (did you mean %ld?)\n", argv[1], secs);
		return(EXIT_FAILURE);
	}
	if (secs <= 0) {
		fprintf(stderr, "duration (%ld) must be greater than zero.\n", secs);
		return(EXIT_FAILURE);
	}

	if (NULL == (win = ui_start())) {
		fprintf(stderr, "could not initialize window\n");
		return(EXIT_FAILURE);
	}

	ctx.window = win;

	timer_start(secs, handler, &ctx);

	if (0 != ui_stop()) {
		fprintf(stderr, "failure stopping window\n");
		return(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}
