#include "pdp11_memory.h"
#include "pdp11_processor.h"

#include "debug.h"
#include "terminal.h"
#include "system.h"

#include <ncurses.h>

#include <stdio.h>

void help(const char *progname)
{
	printf("usage: %s options [arguments]\n", progname);
	// todo: description for every option
}

int main(int argc, char *argv[])
{
	int i, j;

	if (argc == 1) {
		help(argv[0]);
		return SYSTEM_ERROR;
	}

	for (i = 0, j = 2; argv[1][i] != '\0'; ++i) {
		switch (argv[1][i]) {
		case 'h':
			help(argv[0]);
			return SYSTEM_OK;
		case 'v':
			printf("%s-"VERSION"\n", argv[0]);
			return SYSTEM_OK;
		case 'f':
			if (j >= argc) {
				fprintf(stderr, "error: not enough arguments\n");
				return SYSTEM_ERROR;
			}
			loadfile(argv[j++]);
			break;
		case 'd':
		case 'D':
			fprintf(stderr, "error: not implemented\n");
			return SYSTEM_ERROR;
		default:
			fprintf(stderr,
				"error: unknown option %c\n", argv[1][i]);
			return SYSTEM_ERROR;
		}
	}

	if (j < argc) {
		fprintf(stderr, "error: too many arguments\n");
		return SYSTEM_ERROR;
	}

	initscr();

	debug_create();
	terminal_create();

	run();

	terminal_destroy();
	debug_destroy();

	endwin();

	return SYSTEM_OK;
}
