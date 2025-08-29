#ifndef DEBUG_ENABLE

void trace(const char *format, ...)
{
}

#else // DEBUG_ENABLE

#include <stdio.h>
#include <stdarg.h>

void trace(const char *format, ...)
{
	va_list args;
	
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

#endif // DEBUG_ENABLE
