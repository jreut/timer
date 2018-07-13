#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>
#include "timer.h"

#define die(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* we assume end > start, else diff = 0 */
void
time_diff(const struct timespec *start, const struct timespec *end, struct timespec *out)
{
	unsigned long d_sec;
	long d_nsec;
	d_sec = end->tv_sec - start->tv_sec;
	d_nsec = end->tv_nsec - start->tv_nsec;
	if (d_nsec < 0) {
		d_sec--;
		d_nsec += 1000000000;
	}
	if (d_sec > end->tv_sec) {
		d_sec = 0;
		d_nsec = 0;
	}
	out->tv_sec = d_sec;
	out->tv_nsec = d_nsec;
}

void
timer_start(long duration_secs, tick_handler on_tick, void *ctx)
{
	u_int64_t expiration_counter, expirations_per, max_expirations;
	struct timespec now, end, remaining;
	struct itimerspec new_value;
	int fd;
	
	if (clock_gettime(CLOCK_BOOTTIME, &end) != 0)
		die("clock_gettime(end)");
	/* TODO: handle overflow */
	end.tv_sec += duration_secs;
	if ((fd = timerfd_create(CLOCK_BOOTTIME, 0x0)) == -1)
		die("timerfd_create()");
	/* timer expires every second */
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 500000000;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 500000000;
	if (timerfd_settime(fd, 0x0, &new_value, NULL) == -1)
		die("timerfd_settime()");
	expiration_counter = 0;
	max_expirations = duration_secs * 2;
	while (expiration_counter <= max_expirations) {
		if (clock_gettime(CLOCK_BOOTTIME, &now) != 0)
			die("clock_gettime(now)");
		time_diff(&now, &end, &remaining);
		on_tick(&remaining, ctx);
		if (read(fd, &expirations_per, 8) != 8)
			die("read()");
		expiration_counter += expirations_per;
	}
	close(fd);
}
