#include "system.h"
#include "debug.h"
#include "terminal.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>

void system_exit(const char *str, int err)
{
	fprintf(stderr, str);

	terminal_system_destroy();
	debug_system_destroy();

	endwin();

	exit(err);
}
