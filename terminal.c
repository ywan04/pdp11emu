#include "terminal.h"
#include "system.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdarg.h>

static WINDOW *win;

void terminal_create(void)
{
	if (win != NULL) {
		system_exit("error: terminal window is already created\n",
			    SYSTEM_ERROR);
	}
	
	win = newwin(TERMINAL_HEIGHT, TERMINAL_WIDTH, TERMINAL_Y, TERMINAL_X);
	box(win, 0, 0);
	mvwprintw(win, 0, 2, "terminal");
	wmove(win, 1, 1);
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
}

void terminal_putchar(char c)
{
	waddch(win, c);
}

void terminal_getch(void)
{
	wgetch(win);
}

void terminal_refresh(void)
{
	wrefresh(win);
}
