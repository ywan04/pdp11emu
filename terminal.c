#include "terminal.h"
#include "system.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdarg.h>

static WINDOW *win;
static int y, x;

void terminal_create(void)
{
	if (win != NULL) {
		system_exit("error: terminal window is already created\n",
			    SYSTEM_ERROR);
	}
	
	win = newwin(TERMINAL_HEIGHT, TERMINAL_WIDTH, TERMINAL_Y, TERMINAL_X);
	box(win, 0, 0);
	mvwprintw(win, 0, 2, "terminal");
	y = x = 1;
	wmove(win, y, x);
	nodelay(win, TRUE);
	wrefresh(win);
}

void terminal_destroy(void)
{
	if (win != NULL) {
		delwin(win);
		win = NULL;
	} else {
		system_exit("error: terminal window is clean\n", SYSTEM_ERROR);
	}
}

void terminal_system_destroy(void)
{
	if (win != NULL)
		delwin(win);
}

void terminal_clear(void)
{
	terminal_destroy();
	terminal_create();
}

void terminal_putchar(char c)
{
	waddch(win, c);

	++x;
	if (x >= TERMINAL_WIDTH-1) {
		x = 1;
		++y;
		if (y >= TERMINAL_HEIGHT-1) {
			terminal_clear();
			y = 1;
		}
		wmove(win, y, x);
	}

	wrefresh(win);
}

char terminal_getchar(void)
{
	return wgetch(win);
}

void terminal_getch(void)
{
	nodelay(win, FALSE);
	wgetch(win);
	nodelay(win, TRUE);
}

void terminal_refresh(void)
{
	wrefresh(win);
}
