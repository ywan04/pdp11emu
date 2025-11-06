#include "rk11.h"

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

void rk11_attach_disk(uint8_t n, char *filename)
{
	FILE *f;

	n &= 07;

	rk11[n].filename = filename;

	f = fopen(rk11[n].filename, "rb");
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
