CFLAGS = -Wall -Wpedantic -g
LDFLAGS = -lrt

all: compile

compile: timer

timer: timer.c
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}
