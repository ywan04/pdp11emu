#ifndef RK11__H
#define RK11__H

#include <stdint.h>

void rk11_init(void);
void rk11_cycle(void);
uint16_t rk11_readw(uint8_t n, uint8_t c, uint8_t t, uint8_t s, uint8_t a);
void rk11_writew(uint8_t n, uint8_t c, uint8_t t, uint8_t s, uint8_t a,
		 uint16_t val);
void rk11_attach_disk(uint8_t n, char *filename);
void rk11_unattach_disk(uint8_t n);
void rk11_unattach_disks(void);
void rk11_set_read_only(uint8_t n);

#endif /* RK11__H */
