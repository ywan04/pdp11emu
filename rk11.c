#include "rk11.h"
#include "pdp11_memory.h"
#include "system.h"

#include <stdio.h>

struct {
	char *filename;
	uint8_t read_only;

	struct {
	struct {
	struct {
	struct {
		uint16_t data[256];
	} sector[12];
	} track[2];
	} cylinder[203];
	} disk;
} rk11[8];

void rk11_init(void)
{
	writew(A_RKDA, 0000000);
}

void rk11_cycle(void)
{
}

uint16_t rk11_readw(uint8_t n, uint8_t c, uint8_t t, uint8_t s, uint8_t a)
{
	n &= 07;
	c %= 203;
	t %= 2;
	s %= 12;

	return rk11[n].disk.cylinder[c].track[t].sector[s].data[a];
}

void rk11_writew(uint8_t n, uint8_t c, uint8_t t, uint8_t s, uint8_t a,
		 uint16_t val)
{
	n &= 07;
	c %= 203;
	t %= 2;
	s %= 12;

	rk11[n].disk.cylinder[c].track[t].sector[s].data[a] = val;
}

void rk11_attach_disk(uint8_t n, char *filename)
{
	FILE *f;

	n &= 07;

	rk11[n].filename = filename;

	f = fopen(rk11[n].filename, "rb");
	if (f == NULL)
		system_exit("error: cannot open a file", SYSTEM_ERROR);

	fread(&rk11[n].disk, 2, sizeof(rk11[n].disk), f);

	fclose(f);
}

void rk11_unattach_disk(uint8_t n)
{
	FILE *f;

	n &= 07;

	if (rk11[n].filename == NULL)
		return;

	if (!rk11[n].read_only) {
		f = fopen(rk11[n].filename, "wb");
		if (f == NULL)
			system_exit("error: cannot open a file", SYSTEM_ERROR);

		fwrite(&rk11[n].disk, 2, sizeof(rk11[n].disk), f);

		fclose(f);
	}

	rk11[n].filename = NULL;
	rk11[n].read_only = 0;
}

void rk11_unattach_disks(void)
{
	int i;

	for (i = 0; i < 8; ++i)
		rk11_unattach_disk(i);
}

void rk11_set_read_only(uint8_t n)
{
	n &= 07;

	rk11[n].read_only = 1;
}
