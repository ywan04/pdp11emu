#include "pdp11_processor.h"
#include "pdp11_memory.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#define PC reg[7]

struct {
	uint8_t N : 1;
	uint8_t Z : 1;
	uint8_t V : 1;
	uint8_t C : 1;
} flag;

typedef struct {
	uint16_t mask;
	uint16_t opcode;
	char *name;
	void (*do_func)(void);
} instruction_t;

instruction_t ins[] = {
	{ 0177777, 0000000, "halt", do_halt },
	{ 0170000, 0010000, "mov", do_mov },
	{ 0170000, 0060000, "add", do_add },
};

static uint16_t reg[8];
static uint16_t curins;

uint8_t get_src(uint16_t *adr)
{
	uint8_t ss;

	ss = (uint8_t)(curins & 0000077);

	return parse_arg(adr, ss);
}

uint8_t get_dst(uint16_t *adr)
{
	uint8_t dd;
	
	dd = (uint8_t)((curins & 0007700) >> 6);

	return parse_arg(adr, dd);
}

/*
 * Returns 1 if src is register and 0 otherwise
 */
uint8_t parse_arg(uint16_t *adr, uint8_t arg)
{
	uint8_t mod, regn;
	
	mod  = arg >> 3;
	regn = arg & 07;

	switch (mod) {
	case 0:
		*adr = regn;
		return 1;
	case 1:
		*adr = reg[regn];
		return 0;
	case 2:
		*adr = reg[regn];
		reg[regn] += 2;
		return 0;
	case 3:
		*adr = readw(reg[regn]);
		reg[regn] += 2;
		return 0;
	case 4:
		reg[regn] -= 2;
		*adr = reg[regn];
		return 0;
	case 5:
		reg[regn] -= 2;
		*adr = readw(reg[regn]);
		return 0;
	case 6:
		*adr = reg[regn] + readw(PC);
		PC += 2;
		return 0;
	case 7:
		*adr = readw(reg[regn] + readw(PC));
		PC += 2;
		return 0;
	default:
		trace("error: mod cannot be larger than 7\n");
		return 2;
	}
}

void write_arg(uint8_t is_reg, uint16_t adr, uint16_t val)
{
	if (is_reg) {
		r[adr] = val;
	} else {
		writew(adr, val);
	}
}

uint16_t read_arg(uint8_t is_reg, uint16_t adr)
{
	if (is_reg) {
		return r[adr];
	} else {
		return readw(adr);
	}
}

void do_halt(void)
{
	trace("halt.\n");

	exit(0);
}

void do_mov(void)
{
	uint16_t src_adr, dst_adr, val;

	val = read_arg(get_src(&src_adr), src_adr);

	write_arg(get_dst(&dst_adr), dst_adr, val);
	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void do_add(void)
{
	uint16_t src_adr, src_val, src_is_reg;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;

	src_is_reg = get_src(&src_adr);
	src_val = read_arg(src_is_reg, src_adr);

	dst_is_reg = get_dst(&dst_adr);
	dst_val = read_arg(dst_is_reg, dst_adr);

	val = src_val + dst_val;
	write_arg(dst_is_reg, dst_adr, val);
	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = ((val & 0100000) != (dst_val & 0100000));
	flag.C = (val < dst_val);
}

void do_nop(void)
{
}

void run(void)
{
	PC = 01000;

	for (;;) {
		curins = readw(PC);
		trace("%06o %06o: ", PC, curins);
		PC += 2;

		for (i = 0, n = sizeof(ins)/sizeof(instruction_t);
		     i < n;
		     ++i) {
			if ((curins & ins[i].mask) == ins[i].opcode) {
				ins[i].do_func();
			}
		}
	}
}
