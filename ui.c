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
	int x, y;
	getmaxyx(stdscr, y, x);

	if (OK != clear()) return -1;
	if (OK != mvwaddstr(window, y / 2, (x - strlen(str)) / 2, str)) return -1;
	if (OK != refresh()) return -1;
	return 0;

}
