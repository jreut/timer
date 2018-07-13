#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

struct timer_ctx {
	FILE *out;
};

void handler(struct timespec *remaining, void *ctx)
{
	struct timer_ctx *timer_ctx = ctx;
	int secs = remaining->tv_sec - remaining->tv_nsec / 1000000000;
	fprintf(timer_ctx->out, "%02d:%02d\n", secs / 60, secs % 60);
}

int
main(int argc, char *argv[])
{
	long secs;
	char *endptr;
	struct timer_ctx ctx = { .out = stdout };

	if (argc != 2) {
		printf("%s <duration in seconds>\n", argv[0]);
		return EXIT_FAILURE;
	}
	secs = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		printf("%s is invalid (did you mean %ld?)\n", argv[1], secs);
		return EXIT_FAILURE;
	}
	if (secs <= 0) {
		printf("duration (%ld) must be greater than zero.\n", secs);
		return EXIT_FAILURE;
	}
	timer_start(secs, handler, &ctx);
	return EXIT_SUCCESS;
}
