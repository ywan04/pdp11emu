#ifndef SYSTEM__H
#define SYSTEM__H

#include <stdint.h>

#define SYSTEM_OK    0x0
#define SYSTEM_ERROR 0xE

#define SYSTEM_SECOND 1000000000

void system_exit(int err, const char *strf, ...);
void system_start_instruction(void);
void system_finish_instruction(uint64_t nsec);

#endif /* SYSTEM__H */
