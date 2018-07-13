CFLAGS = -Wall -Wpedantic
CFLAGS_DEBUG = ${CFLAGS} -fsanitize=address -g

SRC = main.c timer.c
EXE = timer timer_debug

all: timer

timer_debug: ${SRC}
	${CC} ${CFLAGS_DEBUG} -o $@ $^ ${LDFLAGS}

timer: ${SRC}
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${EXE}
