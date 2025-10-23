#include "pdp11_memory.h"
#include "pdp11_processor.h"

#include "debug.h"
#include "terminal.h"

#include <ncurses.h>

#include <assert.h>

int main(int argc, char *argv[])
{
	initscr();

	debug_create();
	terminal_create();

	//loadfile(argv[1]);
	run();

	terminal_destroy();
	debug_destroy();

	endwin();

	return 0;
}
