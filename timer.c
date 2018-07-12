#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>

#define die(msg)	do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* TODO: we assume end > start */
int
elapsed_time(const struct timespec *start, const struct timespec *end, struct timespec *out)
{
	long d_sec, d_nsec;
	d_sec = end->tv_sec - start->tv_sec;
	d_nsec = end->tv_nsec - start->tv_nsec;
	if (d_nsec < 0) {
		d_sec--;
		d_nsec += 1000000000;
	}
	out->tv_sec = d_sec;
	out->tv_nsec = d_nsec;
	return 0;
}

void
go(long duration_secs)
{
	u_int64_t expiration_counter, expirations_per, max_expirations;
	struct timespec start, now, elapsed;
	struct itimerspec new_value, curr_value;
	int fd;
	
	if (clock_gettime(CLOCK_BOOTTIME, &start) != 0)
		die("clock_gettime(start)");
	if ((fd = timerfd_create(CLOCK_BOOTTIME, 0x0)) == -1)
		die("timerfd_create()");
	/* timer expires every second */
	new_value.it_value.tv_sec = start.tv_sec;
	new_value.it_value.tv_nsec = start.tv_nsec;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 500000000;
	timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL);
	/* timer repeats for one minute */
	expiration_counter = 0;
	max_expirations = duration_secs * 2;
	while (expiration_counter <= max_expirations) {
		if (clock_gettime(CLOCK_BOOTTIME, &now) != 0)
			die("clock_gettime(now)");
		elapsed_time(&start, &now, &elapsed);
		printf("elapsed time: %lu.%ld sec\n",
				elapsed.tv_sec,
				elapsed.tv_nsec);
		if (timerfd_gettime(fd, &curr_value) != 0)
			die("timerfd_gettime()");
		printf("expired %lu of %lu times. next expiry in %lu.%ld sec\n",
				expiration_counter,
				max_expirations,
				curr_value.it_value.tv_sec,
				curr_value.it_value.tv_nsec);
		if (read(fd, &expirations_per, 8) != 8)
			die("read()");
		expiration_counter += expirations_per;
	}
	close(fd);
}

int
main(int argc, char *argv[])
{
	long secs;
	char *endptr;

	if (argc != 2) {
		printf("%s <duration in seconds>\n", argv[0]);
		return EXIT_FAILURE;
	}
	secs = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		printf("%s is invalid (did you mean %ld?\n", argv[1], secs);
		return EXIT_FAILURE;
	}
	if (secs <= 0) {
		printf("duration (%ld) must be greater than zero.\n", secs);
		return EXIT_FAILURE;
	}
	go(secs);
	return EXIT_SUCCESS;
}
