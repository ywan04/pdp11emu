#include "pdp11_memory.h"
#include "pdp11_processor.h"

#include "debug.h"
#include "terminal.h"
#include "system.h"
#include "rk11.h"

#include <ncurses.h>

#include <stdio.h>

void help(const char *progname)
{
	printf("usage: %s options [arguments]\n", progname);
	puts("options:");
	puts("  h\tdisplay help");
	puts("  v\tdisplay version");
	puts("  f\tload file");
	puts("  d\tattach disk");
	puts("  D\tattach disk (READ ONLY)");
	puts("  t\ttrace to file");
}

int main(int argc, char *argv[])
{
	int i, j;
	int n;

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
			if (j+1 > argc) {
				fprintf(stderr,
					"error: not enough arguments\n");
				return SYSTEM_ERROR;
			}
			loadfile(argv[j++]);
			break;
		case 'd':
			if (j+2 > argc) {
				fprintf(stderr,
					"error: not enough arguments\n");
				return SYSTEM_ERROR;
			}
			n = atoi(argv[j++]);
			rk11_attach_disk(n, argv[j++]);
			break;
		case 'D':
			if (j+2 > argc) {
				fprintf(stderr,
					"error: not enough arguments\n");
				return SYSTEM_ERROR;
			}
			n = atoi(argv[j++]);
			rk11_attach_disk(n, argv[j++]);
			rk11_set_read_only(n);
			break;
		case 't':
			if (j+1 > argc) {
				fprintf(stderr,
					"error: not enough arguments\n");
				return SYSTEM_ERROR;
			}
			debug_set_trace(argv[j++]);
			break;
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

	pdp11_run();

	terminal_destroy();
	debug_destroy();

	endwin();

	rk11_unattach_disks();

	return SYSTEM_OK;
}
