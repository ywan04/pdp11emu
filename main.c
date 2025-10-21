#include "pdp11_memory.h"
#include "pdp11_processor.h"

#include <ncurses.h>

#include <assert.h>

int main(int argc, char *argv[])
{
	initscr();

	//loadfile(argv[1]);
	//run();

	getch();
	endwin();

	return 0;
}
