CFLAGS = -Wall -Wpedantic -g

all: compile

compile: timer

timer: timer.c
	${CC} ${CFLAGS} -o $@ $< ${LDFLAGS}
