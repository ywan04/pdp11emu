#include "memory.h"
#include "system.h"
#include "rk11.h"
#include "dl11.h"
#include "mmu.h"

#include <stdio.h>

static union {
	uint16_t words[MEMSIZE/2];
	uint8_t bytes[MEMSIZE];
} memory;

void unibus_init(void)
{
	dl11_init();
	rk11_init();
}

void *get_psw(void)
{
	return &memory.bytes[A_PSW];
}

void mem_addressing(uint32_t padr)
{
	dl11_addressing(padr);
}

uint32_t to_physical(uint16_t adr)
{
	if (mmu_enabled()) {
		return mmu_get_physical(adr);
	}
	if (adr & 0160000)
		return (uint32_t)adr | 0600000;
	return adr;
}

void pwritew(uint32_t padr, uint16_t w)
{
	memory.words[padr/2] = w;
}

uint16_t preadw(uint32_t padr)
{
	return memory.words[padr/2];
}

void writeb(uint16_t adr, uint8_t b)
{
	uint32_t padr;

	padr = to_physical(adr);

	mem_addressing(padr);

	memory.bytes[padr] = b;
}

uint8_t readb(uint16_t adr)
{
	uint32_t padr;

	padr = to_physical(adr);

	mem_addressing(padr);

	return memory.bytes[padr];
}

void writew(uint16_t adr, uint16_t w)
{
	uint32_t padr;

	padr = to_physical(adr);

	if (padr % 2) {
		pdp11_int(004, 7);
		return;
	}

	mem_addressing(padr);

	memory.words[padr/2] = w;
}

uint16_t readw(uint16_t adr)
{
	uint32_t padr;

	padr = to_physical(adr);

	if (padr % 2) {
		pdp11_int(004, 7);
		return;
	}

	mem_addressing(padr);

	return memory.words[padr/2];
}

void loadfile(const char *filename)
{
	FILE *f;
	uint16_t adr, n, i;

	f = fopen(filename, "r");

	if (f == NULL)
		system_exit(SYSTEM_ERROR, "error: cannot open a file '%s'\n",
			    filename);

	fscanf(f, "%hu", &i);

	while (i--) {
		fscanf(f, "%ho %ho\n", &adr, &n);
		writew(adr, n);
	}
}
