#include "pdp11_processor.h"
#include "pdp11_memory.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#define PC reg[7]
#define SP reg[6]

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
	void (*exec)(void);
} instruction_t;

instruction_t ins[] = {

	/* SINGLE OPERAND */

	/* General */
	{ 0177700, 0005000, "clr", clr },
	{ 0177700, 0005100, "com", com },
	{ 0177700, 0005200, "inc", inc },
	{ 0177700, 0005300, "dec", dec },
	{ 0177700, 0005400, "neg", neg },
	{ 0177700, 0005700, "tst", tst },
	/* Rotate & Shift */
	{ 0177700, 0006000, "ror", ror },
	{ 0177700, 0006100, "rol", rol },
	{ 0177700, 0006200, "asr", asr },
	{ 0177700, 0006300, "asl", asl },
	{ 0177700, 0000300, "swab", swab },
	/* Multiple Precision */
	{ 0177700, 0005500, "adc", adc },
	{ 0177700, 0005600, "sbc", sbc },
	{ 0177700, 0006700, "sxt", sxt },

	/* DOUBLE OPERAND */

	/* General */
	{ 0170000, 0010000, "mov", mov },
	{ 0170000, 0020000, "cmp", cmp },
	{ 0170000, 0060000, "add", add },
	{ 0170000, 0160000, "sub", sub },
	/* Logical */
	{ 0170000, 0030000, "bit", bit },
	{ 0170000, 0040000, "bic", bic },
	{ 0170000, 0050000, "bis", bis },
	/* Register */
	{ 0177000, 0070000, "mul", mul },
	{ 0177000, 0071000, "div", div },

	/* BRANCH */

	/* Branches */
	/* Signed Conditional Branches */
	/* Unsigned Conditional Branches */

	/* JUMP & SUBROUTINE */

	{ 0177700, 0000100, "jmp", jmp },
	{ 0177000, 0004000, "jsr", jsr },
	{ 0177770, 0000020, "rts", rts },
	{ 0177000, 0077000, "sob", sob },

	/* TRAP & INTERRUPT */

	/* MISCELLANEOUS */

	{ 0177777, 0000000, "halt", halt },
	{ 0177777, 0000240, "nop", nop },

	/* CONDITION CODE OPERATORS */
};

static uint16_t reg[8];
static uint16_t curins;

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

void clr(void)
{
	uint16_t dst_adr;
	
	if (get_dst(&dst_adr)) {
		reg[dst_adr] = 0;
	} else {
		writew(dst_adr, 0);
	}

	flag.N = flag.V = flag.C = 0;
	flag.Z = 1;
}

void com(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] = ~reg[dst_adr];
	} else {
		val = ~readw(dst_adr);
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
	flag.C = 1;
}

void inc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] = reg[dst_adr] + 1;
	} else {
		val = readw(dst_adr) + 1;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val == 0100000);
}

void dec(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] = reg[dst_adr] - 1;
	} else {
		val = readw(dst_adr) - 1;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val == 0077777);
}

void neg(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		reg[dst_adr] = val = ~reg[dst_adr] + 1;
	} else {
		val = ~readw(dst_adr) + 1;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val == 0100000);
	flag.C = !flag.Z;
}

void tst(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	val = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = flag.C = 0;
}

void ror(void)
{
	uint16_t dst_adr;
	uint16_t dst_is_reg;
	uint16_t val;
	uint16_t new_c;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	
	new_c = val & 01;
	val >>= 1;
	if (flag.C) {
		val |= 0100000;
	}

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (new_c != flag.C);
	flag.C = new_c;
}

void rol(void)
{
	uint16_t dst_adr;
	uint16_t dst_is_reg;
	uint16_t val;
	uint16_t new_c;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	new_c = (val & 0100000) >> 15;
	val <<= 1;
	if (flag.C) {
		val |= 01;
	}

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (new_c != flag.C);
	flag.C = new_c;
}

void asr(void)
{
	uint16_t dst_adr;
	uint16_t dst_is_reg;
	uint16_t new_c;
	int16_t val;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	new_c = val & 01;
	val >>= 1;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = flag.N ^ new_c;
	flag.C = new_c;
}

void asl(void)
{
	uint16_t dst_adr;
	uint16_t dst_is_reg;
	uint16_t new_c;
	int16_t val;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	new_c = (val & 0100000) >> 15;
	val <<= 1;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = flag.N ^ new_c;
	flag.C = new_c;
}

void swab(void)
{
	uint16_t dst_adr;
	uint16_t dst_is_reg;
	uint16_t val, hval;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);
	
	hval = (val & ~(uint16_t)0377) >> 8;
	val <<= 8;
	val |= hval;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((hval & 0000200) != 0);
	flag.Z = (hval == 0);
	flag.V = flag.C = 0;
}

void adc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] += flag.C;
	} else {
		val = readw(dst_adr) + flag.C;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val == 0100000 && flag.C);
	flag.C = (flag.Z && flag.C);
}

void sbc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] -= flag.C;
	} else {
		val = readw(dst_adr) - flag.C;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val == 0077777 && flag.C);
	flag.C = (val == 0177777 && flag.C);
}

void sxt(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] = (flag.N) ? 0177777 : 0;
	} else {
		val = (flag.N) ? 0177777 : 0;
		writew(dst_adr, val);
	}

	flag.Z = (val == 0);
}

void halt(void)
{
	trace("halt.\n");

	exit(0);
}

void mov(void)
{
	uint16_t src_adr, dst_adr, val;

	val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);

	if (get_dst(&dst_adr)) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void cmp(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;
	uint16_t neg_bit, src_neg_bit, dst_neg_bit;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);

	val = src_val - dst_val;

	neg_bit = val & 0100000;
	src_neg_bit = src_val & 0100000;
	dst_neg_bit = dst_val & 0100000;

	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (src_neg_bit != dst_neg_bit)
		&& (dst_neg_bit == neg_bit);
	flag.C = (dst_val > src_val);
}

void add(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;
	uint16_t neg_bit, src_neg_bit, dst_neg_bit;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	val = src_val + dst_val;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	neg_bit = val & 0100000;
	src_neg_bit = src_val & 0100000;
	dst_neg_bit = dst_val & 0100000;

	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (src_neg_bit == dst_neg_bit)
		&& (dst_neg_bit != neg_bit);
	flag.C = (val < dst_val);
}

void sub(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;
	uint16_t neg_bit, src_neg_bit, dst_neg_bit;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	val = dst_val - src_val;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	neg_bit = val & 0100000;
	src_neg_bit = src_val & 0100000;
	dst_neg_bit = dst_val & 0100000;

	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (src_neg_bit != dst_neg_bit)
		&& (src_neg_bit == neg_bit);
	flag.C = (src_val > dst_val);
}

void bit(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);

	val = src_val & dst_val;

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void bic(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	val = ~src_val & dst_val;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void bis(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	val = src_val | dst_val;

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void mul(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint32_t val;
	uint8_t r;

	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);
	r = (curins & 0000700) >> 6;

	val = (uint32_t)reg[r] * src_val;
	
	if (r % 2) {
		reg[r] = val;

		flag.Z = (reg[r] == 0);
		flag.C = (val != reg[r]);
	} else {
		*(uint32_t *)&reg[r] = val;

		flag.Z = (val == 0);
		flag.C = (val != reg[r+1]);
	}

	flag.N = ((reg[r] & 0100000) != 0);
	flag.V = 0;
}

void div(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint16_t val;
	uint16_t rem;
	uint8_t r;

	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);
	r = (curins & 0000700) >> 6;

	if (r % 2) {
		exit(1);
	}

	flag.V = 1;
	flag.C = 1;
	if (src_val != 0) {
		val = *(uint32_t *)&reg[r] / src_val;
		rem = *(uint32_t *)&reg[r] % src_val;

		if (abs(*(uint32_t *)&reg[r]) <= abs(src_val)) {
			flag.V = 0;
		}

		reg[r] = val;
		reg[r+1] = rem;

		flag.C = 0;
	}
	
	flag.N = ((reg[r] & 0100000) != 0);
	flag.Z = (reg[r] == 0);
}

void jmp(void)
{
	uint16_t dst_adr;

	if (get_dst(&dst_adr)) {
		PC = reg[dst_adr];
	} else {
		PC = readw(dst_adr);
	}
}

void jsr(void)
{
	uint16_t dst_adr;
	uint16_t r;

	r = (curins & 0000700) >> 6;

	SP -= 2;
	writew(SP, reg[r]);
	reg[r] = PC;
	PC = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);
}

void rts(void)
{
	uint16_t r;

	r = curins & 07;

	PC = reg[r];
	reg[r] = readw(SP);
	SP +=2;
}

void sob(void)
{
	uint16_t r, nn;

	nn = curins & 0000077;
	r = (curins & 0000700) >> 6;

	--reg[r];

	if (reg[r])
		PC -= 2 * nn;
}

void nop(void)
{
}

void run(void)
{
	uint8_t i, n;
	
	PC = 01000;

	for (;;) {
		curins = readw(PC);
		trace("%06o %06o: ", PC, curins);
		PC += 2;

		for (i = 0, n = sizeof(ins)/sizeof(instruction_t);
		     i < n;
		     ++i) {
			if ((curins & ins[i].mask) == ins[i].opcode) {
				ins[i].exec();
			}
		}
	}
}
