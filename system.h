#ifndef SYSTEM__H
#define SYSTEM__H

#include <stdint.h>

#define SYSTEM_OK    0x0
#define SYSTEM_ERROR 0xE

#define SYSTEM_SECOND 1000000000

void system_exit(int err, const char *strf, ...);
void system_nsleep(uint64_t nsec);
uint64_t system_gettime(void);

#endif /* SYSTEM__H */
