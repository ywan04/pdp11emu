#include "system.h"
#include "debug.h"
#include "terminal.h"
#include "rk11.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>

void system_exit(int err, const char *strf, ...)
{
	va_list args;

	terminal_system_destroy();
	debug_system_destroy();

	endwin();

	rk11_unattach_disks();

	va_start(args, strf);
	vfprintf(stderr, strf, args);
	va_end(args);

	exit(err);
}
