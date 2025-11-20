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
	uint16_t rkcs_data, rkda_data, go, func;
	uint16_t n, cyl, sur, sec, cw;
	uint16_t adr;
	int16_t wcr;

	rkcs_data = readw(A_RKCS);
	go = rkcs_data & 01;
	func = (rkcs_data >> 1) & 07;

	rkda_data = readw(A_RKDA);
	n = rkda_data >> 13;
	cyl = (rkda_data >> 5) & 0377;
	sur = (rkda_data >> 4) & 01;
	sec = rkda_data & 017;
	cw = 0;

	wcr = readw(A_RKWC);
	adr = readw(A_RKBA);

	if (go) {
		rkcs_data &= 0177776;

		switch (func) {
		case RK11_CONTROL_RESET:
			break;
		case RK11_WRITE:
			break;
		case RK11_READ:
			for (; wcr < 0; ++wcr, adr += 2) {
				writew(adr, rk11_readw(n, cyl, sur, sec, cw++));
				if (cw >= 256) {
					cw = 0;
					++sec;
				}
				if (sec >= 12) {
					sec = 0;
					++sur;
				}
				if (sur >= 2) {
					sur = 0;
					++cyl;
				}
				if (cyl >= 203) {
					// todo
				}
			}
			break;
		case RK11_WRITE_CHECK:
			break;
		case RK11_SEEK:
			break;
		case RK11_READ_CHECK:
			break;
		case RK11_DRIVE_RESET:
			break;
		case RK11_WRITE_LOCK:
			break;
		}

		rkcs_data |= 0200;
		writew(A_RKCS, rkcs_data);
	}
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
		system_exit(SYSTEM_ERROR, "error: cannot open a file '%s'\n",
			    rk11[n].filename);

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
			system_exit(SYSTEM_ERROR,
				    "error: cannot open a file '%s'\n",
				    rk11[n].filename);

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
