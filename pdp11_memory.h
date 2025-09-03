#ifndef PDP11_MEMORY__H
#define PDP11_MEMORY__H

#define MEMSIZE (128*1024) // 128KB

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned short address_t;

void writeb(address_t addr, byte_t b);
byte_t readb(address_t addr);
void writew(address_t addr, word_t w);
word_t readw(address_t addr);
void loadfile(const char *filename);

#endif // PDP11_MEMORY__H
