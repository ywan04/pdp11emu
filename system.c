#include "system.h"
#include "debug.h"
#include "terminal.h"
#include "rk11.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>

void system_exit(const char *str, int err)
{
	terminal_system_destroy();
	debug_system_destroy();

	endwin();

	rk11_unattach_disks();

	fprintf(stderr, str);

	exit(err);
}
