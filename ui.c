#include <locale.h>
#include <string.h>
#include "ui.h"

WINDOW *
ui_start()
{
	if (NULL == setlocale(LC_ALL, ""))
		return(NULL);
	if (NULL == initscr())
		return(NULL);
	cbreak();
	noecho();
	curs_set(0);
	clear();
	refresh();
	return stdscr;
}

int
ui_stop(WINDOW *window)
{
	if (ERR == endwin()) return -1;
	return 0;
}

int
ui_set_centered(WINDOW *window, char *str)
{
	int x, y, cX, cY;
	getmaxyx(stdscr, y, x);
	cX = (x - strlen(str)) / 2;
	cY = y / 2;

	if (OK != clear()) return -1;
	if (OK != mvwaddstr(window, cY, cX, str))
		return -1;
	if (OK != refresh()) return -1;
	return 0;

}
