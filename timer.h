#ifndef TIMER_H
#define TIMER_H
typedef void(*tick_handler)(struct timespec *, void *, int error);

void timer_start(long, tick_handler, void *);
#endif
