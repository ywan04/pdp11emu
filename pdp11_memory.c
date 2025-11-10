#include "pdp11_memory.h"
#include "system.h"
#include "rk11.h"

#include <stdio.h>

static union {
	uint16_t words[MEMSIZE/2];
	uint8_t bytes[MEMSIZE];
} memory;

static char rbuf_r;

void unibus_init(void)
{
	writew(A_XCSR, 0000200); /* XCSR: transmitter ready */
	writew(A_XBUF, 0000000);
	writew(A_RCSR, 0000000);
	writew(A_RBUF, 0000000);
	rbuf_readed();

	rk11_init();
}

void *get_low_psw(void)
{
	return &memory.bytes[A_PSW+1];
}

char rbuf_readed(void)
{
	if (rbuf_r) {
		rbuf_r = 0;
		return 1;
	}

	return 0;
}

void mem_addressing(uint16_t adr)
{
	if (adr == A_RBUF)
		rbuf_r = 1;
}

void writeb(uint16_t adr, uint8_t b)
{
	mem_addressing(adr);

	memory.bytes[adr] = b;
}

uint8_t readb(uint16_t adr)
{
	mem_addressing(adr);

	return memory.bytes[adr];
}

void writew(uint16_t adr, uint16_t w)
{
	mem_addressing(adr);

	memory.words[adr/2] = w;
}

uint16_t readw(uint16_t adr)
{
	mem_addressing(adr);

	return memory.words[adr/2];
}

void loadfile(const char *filename)
{
	FILE *f;
	uint16_t adr, n, i;

	f = fopen(filename, "r");

	if (f == NULL)
		system_exit(SYSTEM_ERROR, "error: cannot open a file %s\n",
			    filename);

	fscanf(f, "%hu", &i);

	while (i--) {
		fscanf(f, "%ho %ho\n", &adr, &n);
		writew(adr, n);
	}
}
