#include "system.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

void system_exit(const char *str, int err)
{
	fprintf(stderr, str);

	debug_silent_destroy();
	
	exit(err);
}
