#include "debug.h"
#include "system.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdarg.h>

static WINDOW *win;
static FILE *tfile;

void debug_create(void)
{
	if (win != NULL) {
		system_exit(SYSTEM_ERROR,
			    "error: debug window is already created\n");
	}
	
	win = newwin(DEBUG_HEIGHT, DEBUG_WIDTH, DEBUG_Y, DEBUG_X);
	box(win, 0, 0);
	mvwprintw(win, 0, 2, "debug");
}

void debug_destroy(void)
{
	debug_close_trace();
	if (win != NULL) {
		delwin(win);
		win = NULL;
	} else {
		system_exit(SYSTEM_ERROR, "error: debug window is clean\n");
	}
}

void debug_system_destroy(void)
{
	if (win != NULL)
		delwin(win);
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
	int i;

	wmove(win, DEBUG_HEIGHT-4, 5);

	for (i = 5; i < DEBUG_WIDTH-1; ++i)
		waddch(win, ' ');

	wmove(win, DEBUG_HEIGHT-4, 5);
}

void debug_print(const char *format, ...)
{
	char str[128];
	va_list args;

	va_start(args, format);
	vsnprintf(str, 128, format, args);
	wprintw(win, str);
	if (tfile != NULL)
		fprintf(tfile, str);
	va_end(args);
}

void debug_refresh(void)
{
	wrefresh(win);
	if (tfile != NULL)
		fputc('\n', tfile);
}

void debug_set_trace(const char *filename)
{
	if (tfile != NULL)
		fclose(tfile);

	tfile = fopen(filename, "w");
}

void debug_close_trace(void)
{
	if (tfile != NULL) {
		fclose(tfile);
		tfile = NULL;
	}
}
