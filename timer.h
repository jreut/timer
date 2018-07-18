#ifndef TIMER_H
#define TIMER_H
typedef void(*tick_handler)(struct timespec *, void *);

void timer_start(long, tick_handler, void *);
#endif
