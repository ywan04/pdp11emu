#include "pdp11.h"

#include <stdio.h>

static union {
	word_t words[MEMSIZE/2];
	byte_t bytes[MEMSIZE];
} memory;

word_t reg[8];

void writeb(address_t addr, byte_t b)
{
	memory.bytes[addr] = b;
}

byte_t readb(address_t addr)
{
	return memory.bytes[addr];
}

void writew(address_t addr, word_t w)
{
	memory.words[addr/2] = w;
}

word_t readw(address_t addr)
{
	return memory.words[addr/2];
}

void loadfile(const char *filename)
{
	FILE *f;
	address_t addr, n, i;
	byte_t b;

	f = fopen(filename, "rb");

	fread(&addr, sizeof(address_t), 1, f);
	fread(&n, sizeof(address_t), 1, f);
	for (i = 0; i < n; ++i) {
		fread(&b, 1, 1, f);
		writeb(addr+i, b);
	}
}
