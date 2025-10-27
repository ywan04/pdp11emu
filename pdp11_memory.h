#ifndef PDP11_MEMORY__H
#define PDP11_MEMORY__H

#include <stdint.h>

#define MEMSIZE (64*1024) // 64KB

#define A_RCSR 0177560
#define A_RBUF 0177562
#define A_XCSR 0177564
#define A_XBUF 0177566

char rbuf_readed(void);
void mem_addressing(uint16_t adr);
void writeb(uint16_t adr, uint8_t b);
uint8_t readb(uint16_t adr);
void writew(uint16_t adr, uint16_t w);
uint16_t readw(uint16_t adr);
void loadfile(const char *filename);

#endif // PDP11_MEMORY__H
