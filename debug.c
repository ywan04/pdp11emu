#include "debug.h"
#include "system.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdarg.h>

static WINDOW *win;

void debug_create(void)
{
	if (win != NULL) {
		system_exit("error: debug window is already created\n",
			    SYSTEM_ERROR);
	}
	
	win = newwin(DEBUG_HEIGHT, DEBUG_WIDTH, DEBUG_Y, DEBUG_X);
	box(win, 0, 0);
	mvwprintw(win, 0, 2, "debug");
}

void debug_destroy(void)
{
	if (win != NULL) {
		delwin(win);
		win = NULL;
	} else {
		system_exit("error: debug window is clean\n", SYSTEM_ERROR);
	}
}

void debug_silent_destroy(void)
{
	if (win != NULL) {
		delwin(win);
		win = NULL;
	}
}

void debug_print_regs(uint16_t *reg)
{
	int i;

	for (i = 0; i < 8; i += 2) {
		mvwprintw(win, i/2+2, (DEBUG_WIDTH-2*(4+6))/3, "R%d: %06o", i, reg[i]);
	}
	for (i = 1; i < 8; i += 2) {
		mvwprintw(win, i/2+2, (2*DEBUG_WIDTH-(4+6))/3, "R%d: %06o", i, reg[i]);
	}
}

void debug_print_init(void)
{
	wmove(win, DEBUG_HEIGHT-3, 7);
}

void debug_print(const char *format, ...)
{
	char str[128];
	va_list args;

	va_start(args, format);
	vsnprintf(str, 128, format, args);
	wprintw(win, str);
	va_end(args);
}

void debug_refresh(void)
{
	wrefresh(win);
}
