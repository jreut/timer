CFLAGS = -Wall -Wpedantic
CFLAGS_DEBUG = ${CFLAGS} -fsanitize=address -g
LDFLAGS = -lncursesw

SRC = main.c timer.c ui.c
EXE = timer timer_debug

all: timer

timer_debug: ${SRC}
	${CC} ${CFLAGS_DEBUG} -o $@ $^ ${LDFLAGS}

timer: ${SRC}
	${CC} ${CFLAGS} -o $@ $^ ${LDFLAGS}

clean:
	rm -f ${EXE} *.o
