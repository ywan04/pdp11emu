#include "pdp11_memory.h"

#include <stdio.h>

static union {
	uint16_t words[MEMSIZE/2];
	uint8_t bytes[MEMSIZE];
} memory;

void writeb(uint16_t adr, uint8_t b)
{
	memory.bytes[adr] = b;
}

uint8_t readb(uint16_t adr)
{
	return memory.bytes[adr];
}

void writew(uint16_t adr, uint16_t w)
{
	memory.words[adr/2] = w;
}

uint16_t readw(uint16_t adr)
{
	return memory.words[adr/2];
}

void loadfile(const char *filename)
{
	FILE *f;
	uint16_t adr, n, i;

	f = fopen(filename, "r");

	fscanf(f, "%hu", &i);

	while (i--) {
		fscanf(f, "%ho %ho\n", &adr, &n);
		writew(adr, n);
	}
}
