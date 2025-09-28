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
	void (*do_func)(void);
} instruction_t;

instruction_t ins[] = {
	
	/* SINGLE OPERAND */
	
	/* General */
	{ 0177700, 0005000, "clr", do_clr },
	{ 0177700, 0005100, "com", do_com },
	{ 0177700, 0005200, "inc", do_inc },
	{ 0177700, 0005300, "dec", do_dec },
	{ 0177700, 0005400, "neg", do_neg },
	{ 0177700, 0005700, "tst", do_tst },
	/* Rotate & Shift */
	{ 0177700, 0006000, "ror", do_ror },
	{ 0177700, 0006100, "rol", do_rol },
	/* Multiple Precision */

	/* DOUBLE OPERAND */
	
	/* General */
	{ 0170000, 0010000, "mov", do_mov },
	{ 0170000, 0020000, "cmp", do_cmp },
	{ 0170000, 0060000, "add", do_add },
	{ 0170000, 0160000, "sub", do_sub },
	/* Logical */
	/* Register */

	/* BRANCH */

	/* Branches */
	/* Signed Conditional Branches */
	/* Unsigned Conditional Branches */

	/* JUMP & SUBROUTINE */

	{ 0177700, 0000100, "jmp", do_jmp },
	{ 0177000, 0004000, "jsr", do_jsr },
	{ 0177770, 0000020, "rts", do_rts },
	{ 0177000, 0077000, "sob", do_sob },

	/* TRAP & INTERRUPT */

	/* MISCELLANEOUS */

	{ 0177777, 0000000, "halt", do_halt },
	{ 0177777, 0000240, "nop", do_nop },

	/* CONDITION CODE OPERATORS */
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

void do_clr(void)
{
	uint16_t dst_adr;
	
	if (get_dst(&dst_adr)) {
		r[dst_adr] = 0;
	} else {
		writew(dst_adr, 0);
	}

	flag.N = flag.V = flag.C = 0;
	flag.Z = 1;
}

void do_com(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = r[dst_adr] = ~r[dst_adr];
	} else {
		val = ~readw(dst_adr);
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
	flag.C = 1;
}

void do_inc(void)
{
	uint16_t dst_adr;
	uint16_t dst_val, val;

	if (get_dst(&dst_adr)) {
		dst_val = r[dst_adr];
		val = r[dst_adr] = dst_val + 1;
	} else {
		dst_val = readw(dst_adr);
		val = dst_val + 1;
		writew(dst_adr, val);
	}

	val_neg_bit = val & 0100000;
	dst_val_neg_bit = dst_val & 0100000;
	
	flag.N = (val_neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (val_neg_bit != dst_val_neg_bit);
}

void do_dec(void)
{
	uint16_t dst_adr;
	uint16_t dst_val, val;
	uint16_t val_neg_bit, dst_val_neg_bit;
	
	if (get_dst(&dst_adr)) {
		dst_val = r[dst_adr];
		val = r[dst_adr] = dst_val - 1;
	} else {
		dst_val = readw(dst_adr);
		val = dst_val - 1;
		writew(dst_adr, val);
	}

	val_neg_bit = val & 0100000;
	dst_val_neg_bit = dst_val & 0100000;
	
	flag.N = (val_neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (val_neg_bit != dst_val_neg_bit);
}

void do_neg(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		r[dst_adr] = val = ~r[dst_adr] + 1;
	} else {
		val = ~readw(dst_adr) + 1;
		writew(dst_adr, val);
	}

	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (val != 0);
	flag.C = flag.Z;
}

void do_tst(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = r[dst_adr];
	} else {
		val = readw(dst_adr);
	}

	val_neg_bit = val & 0100000;
	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
}

void do_ror(void)
{
	uint16_t dst_adr;
	uint16_t val;
	uint16_t new_c;
	
	if (get_dst(&dst_adr)) {
		val = r[dst_adr];
	} else {
		val = readw(dst_adr);
	}

	new_c = val & 01;
	val >>= 1;
	if (flag.C) {
		val |= 0100000;
	}

	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (new_c != flag.C);
	flag.C = new_c;
}

void do_rol(void)
{
	uint16_t dst_adr;
	uint16_t val;
	uint16_t new_c;
	
	if (get_dst(&dst_adr)) {
		val = r[dst_adr];
	} else {
		val = readw(dst_adr);
	}

	new_c = (val & 0100000) >> 15;
	val <<= 1;
	if (flag.C) {
		val |= 01;
	}

	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = (new_c != flag.C);
	flag.C = new_c;
}

void do_halt(void)
{
	trace("halt.\n");

	exit(0);
}

void do_mov(void)
{
	uint16_t src_adr, dst_adr, val;

	val = (get_src(&src_adr)) ? r[src_adr] : readw(src_adr);

	if (get_dst(&dst_adr)) {
		r[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}
	
	flag.N = ((val & 0100000) != 0);
	flag.Z = (val == 0);
	flag.V = 0;
}

void do_cmp(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;
	uint16_t neg_bit;

	src_val = (get_src(&src_adr)) ? r[src_adr] : readw(src_adr);
	dst_val = (get_dst(&dst_adr)) ? r[dst_adr] : readw(dst_adr);
	val = src_val - dst_val;

	neg_bit = val & 0100000;
	
	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = (neg_bit != (src_val & 0100000))
		&& (neg_bit != (dst_val & 0100000));
	flag.C = (dst_val > src_val);
}

void do_add(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;
	uint16_t neg_bit;
	
	src_val = (get_src(&src_adr)) ? r[src_adr] : readw(src_adr);

	dst_is_reg = get_dst(&dst_adr);
	dst_val = (dst_is_reg) ? r[dst_adr] : readw(dst_adr);

	val = src_val + dst_val;
	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	neg_bit = val & 0100000;
	
	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = ((neg_bit != (dst_val & 0100000))
		  && (neg_bit != (src_val & 0100000)));
	flag.C = (val < dst_val);
}

void do_sub(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val, dst_is_reg;
	uint16_t val;
	uint16_t neg_bit;
	
	src_val = (get_src(&src_adr)) ? r[src_adr] : readw(src_adr);

	dst_is_reg = get_dst(&dst_adr);
	dst_val = (dst_is_reg) ? r[dst_adr] : readw(dst_adr);

	val = dst_val - src_val;
	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	neg_bit = val & 0100000;
	
	flag.N = (neg_bit != 0);
	flag.Z = (val == 0);
	flag.V = ((neg_bit != (dst_val & 0100000))
		  && (neg_bit != (src_val & 0100000)));
	flag.C = (src_val > dst_val);
}

void do_jmp(void)
{
	uint16_t dst_adr;

	if (get_dst(&dst_adr)) {
		PC = reg[dst_adr];
	} else {
		PC = readw(dst_adr);
	}
}

void do_jsr(void)
{
	uint16_t dst_adr;
	uint16_t r;

	r = (curins & 0000700) >> 6;

	SP -= 2;
	writew(SP, reg[r]);
	reg[r] = PC;
	PC = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);
}

void do_rts(void)
{
	uint16_t r;

	r = curins & 07;

	PC = reg[r];
	reg[r] = readw(SP);
	SP +=2;
}

void do_sob(void)
{
	uint16_t r, nn;

	nn = curins & 0000077;
	r = (curins & 0000700) >> 6;

	--reg[r];

	if (reg[r])
		PC = PC - 2 * nn;
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
