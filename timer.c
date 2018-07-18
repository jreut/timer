#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/timerfd.h>
#include "timer.h"

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
	uint64_t expiration_counter, expirations_per, max_expirations;
	struct timespec now, end, remaining;
	struct itimerspec new_value;
	int fd;
	
	clock_gettime(CLOCK_BOOTTIME, &end);
	/* TODO: handle overflow */
	end.tv_sec += duration_secs;
	fd = timerfd_create(CLOCK_BOOTTIME, 0x0);
	/* timer expires every second */
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_nsec = 500000000;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 500000000;
	timerfd_settime(fd, 0x0, &new_value, NULL);
	expiration_counter = 0;
	max_expirations = duration_secs * 2;
	while (expiration_counter <= max_expirations) {
		clock_gettime(CLOCK_BOOTTIME, &now);
		time_diff(&now, &end, &remaining);
		on_tick(&remaining, ctx);
		read(fd, &expirations_per, 8);
		expiration_counter += expirations_per;
	}
	close(fd);
}
