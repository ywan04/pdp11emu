#ifndef MEMORY__H
#define MEMORY__H

#include <stdint.h>

#define MEMSIZE (256*1024) /* 256KB */

#define A_PSW  0177776

#define A_RCSR 0177560
#define A_RBUF 0177562
#define A_XCSR 0177564
#define A_XBUF 0177566

#define A_RKDS 0177400
#define A_RKER 0177402
#define A_RKCS 0177404
#define A_RKWC 0177406
#define A_RKBA 0177410
#define A_RKDA 0177412

void unibus_init(void);
void *get_low_psw(void);
char rbuf_readed(void);
void mem_addressing(uint32_t adr);
void writeb(uint16_t adr, uint8_t b);
uint8_t readb(uint16_t adr);
void writew(uint16_t adr, uint16_t w);
uint16_t readw(uint16_t adr);
void loadfile(const char *filename);

#endif /* MEMORY__H */
