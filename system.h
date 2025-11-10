#ifndef SYSTEM__H
#define SYSTEM__H

#define SYSTEM_OK    0x0
#define SYSTEM_ERROR 0xE

void system_exit(int err, const char *strf, ...);

#endif /* SYSTEM__H */
