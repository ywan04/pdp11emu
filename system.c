#include "system.h"
#include "debug.h"
#include "terminal.h"
#include "rk11.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>

static uint64_t start_time;

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

void nsleep(uint64_t nsec)
{
	struct timespec t;

	t.tv_sec  = nsec / SYSTEM_SECOND;
	t.tv_nsec = nsec % SYSTEM_SECOND;

	nanosleep(&t, NULL);
}

uint64_t gettime(void)
{
	struct timespec t;
	uint64_t nsec;

	clock_gettime(CLOCK_REALTIME, &t);

	nsec  = t.tv_sec * SYSTEM_SECOND;
	nsec += t.tv_nsec;

	return nsec;
}

void system_start_instruction(void)
{
	start_time = gettime();
}

void system_finish_instruction(uint64_t ins_time)
{
	uint64_t delta_time;

	delta_time = gettime() - start_time;
	if (delta_time < ins_time)
		nsleep(ins_time - delta_time);
}
