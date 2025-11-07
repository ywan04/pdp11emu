#ifndef RK11__H
#define RK11__H

#include <stdint.h>

enum rk11_functions {
	RK11_CONTROL_RESET,
	RK11_WRITE,
	RK11_READ,
	RK11_WRITE_CHECK,
	RK11_SEEK,
	RK11_READ_CHECK,
	RK11_DRIVE_RESET,
	RK11_WRITE_LOCK
};

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
