#ifndef PDP11_MEMORY__H
#define PDP11_MEMORY__H

#include <stdint.h>

#define MEMSIZE (64*1024) // 64KB

void writeb(uint16_t addr, uint8_t b);
uint8_t readb(uint16_t addr);
void writew(uint16_t addr, uint16_t w);
uint16_t readw(uint16_t addr);
void loadfile(const char *filename);

#endif // PDP11_MEMORY__H
