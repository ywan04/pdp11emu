#ifndef RK11__H
#define RK11__H

#include <stdint.h>

void rk11_init(void);
void rk11_attach_disk(uint8_t n, char *filename);
void rk11_unattach_disk(uint8_t n);
void rk11_unattach_disks(void);
void rk11_set_read_only(uint8_t n);

#endif /* RK11__H */
