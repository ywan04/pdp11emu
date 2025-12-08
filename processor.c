#include "processor.h"
#include "memory.h"
#include "debug.h"
#include "terminal.h"
#include "system.h"
#include "rk11.h"

#include <ncurses.h>

#include <stdio.h>
#include <stdlib.h>

#define PC reg[7]
#define SP reg[6]

struct {
	uint8_t P : 3;
	uint8_t T : 1;
	uint8_t N : 1;
	uint8_t Z : 1;
	uint8_t V : 1;
	uint8_t C : 1;
} *flag;

typedef struct {
	uint16_t mask;
	uint16_t opcode;
	char *name;
	void (*exec)(void);
} instruction_t;

instruction_t ins[] = {

	/* SINGLE OPERAND */

	/* General */
	{ 0177700, 0005000, "clr", p_clr },
	{ 0177700, 0105000, "clrb", p_clrb },
	{ 0177700, 0005100, "com", p_com },
	{ 0177700, 0105100, "comb", p_comb },
	{ 0177700, 0005200, "inc", p_inc },
	{ 0177700, 0105200, "incb", p_incb },
	{ 0177700, 0005300, "dec", p_dec },
	{ 0177700, 0105300, "decb", p_decb },
	{ 0177700, 0005400, "neg", p_neg },
	{ 0177700, 0105400, "negb", p_negb },
	{ 0177700, 0005700, "tst", p_tst },
	{ 0177700, 0105700, "tstb", p_tstb },
	/* Rotate & Shift */
	{ 0177700, 0006000, "ror", p_ror },
	{ 0177700, 0106000, "rorb", p_rorb },
	{ 0177700, 0006100, "rol", p_rol },
	{ 0177700, 0106100, "rolb", p_rolb },
	{ 0177700, 0006200, "asr", p_asr },
	{ 0177700, 0106200, "asrb", p_asrb },
	{ 0177700, 0006300, "asl", p_asl },
	{ 0177700, 0106300, "aslb", p_aslb },
	{ 0177700, 0000300, "swab", p_swab },
	/* Multiple Precision */
	{ 0177700, 0005500, "adc", p_adc },
	{ 0177700, 0105500, "adcb", p_adcb },
	{ 0177700, 0005600, "sbc", p_sbc },
	{ 0177700, 0105600, "sbcb", p_sbcb },
	{ 0177700, 0006700, "sxt", p_sxt },

	/* DOUBLE OPERAND */

	/* General */
	{ 0170000, 0010000, "mov", p_mov },
	{ 0170000, 0110000, "movb", p_movb },	
	{ 0170000, 0020000, "cmp", p_cmp },
	{ 0170000, 0120000, "cmpb", p_cmpb },
	{ 0170000, 0060000, "add", p_add },
	{ 0170000, 0160000, "sub", p_sub },
	/* Logical */
	{ 0170000, 0030000, "bit", p_bit },
	{ 0170000, 0130000, "bitb", p_bitb },
	{ 0170000, 0040000, "bic", p_bic },
	{ 0170000, 0140000, "bicb", p_bicb },
	{ 0170000, 0050000, "bis", p_bis },
	{ 0170000, 0150000, "bisb", p_bisb },
	/* Register */
	{ 0177000, 0070000, "mul", p_mul },
	{ 0177000, 0071000, "div", p_div },
	{ 0177000, 0072000, "ash", p_ash },
	{ 0177000, 0073000, "ashc", p_ashc },
	{ 0177000, 0074000, "xor", p_xor },

	/* BRANCH */

	/* Branches */
	{ 0177400, 0000400, "br", p_br },
	{ 0177400, 0001000, "bne", p_bne },
	{ 0177400, 0001400, "beq", p_beq },
	{ 0177400, 0100000, "bpl", p_bpl },
	{ 0177400, 0100400, "bmi", p_bmi },
	{ 0177400, 0102000, "bvc", p_bvc },
	{ 0177400, 0102400, "bvs", p_bvs },
	{ 0177400, 0103000, "bcc/bhis", p_bcc },
	{ 0177400, 0103400, "bcs/blo", p_bcs },
	/* Signed Conditional Branches */
	{ 0177400, 0002000, "bge", p_bge },
	{ 0177400, 0002400, "blt", p_blt },
	{ 0177400, 0003000, "bgt", p_bgt },
	{ 0177400, 0003400, "ble", p_ble },
	/* Unsigned Conditional Branches */
	{ 0177400, 0101000, "bhi", p_bhi },
	{ 0177400, 0101400, "blo", p_blo },

	/* JUMP & SUBROUTINE */

	{ 0177700, 0000100, "jmp", p_jmp },
	{ 0177000, 0004000, "jsr", p_jsr },
	{ 0177770, 0000200, "rts", p_rts },
	{ 0177700, 0006400, "mark", p_mark },
	{ 0177000, 0077000, "sob", p_sob },

	/* TRAP & INTERRUPT */

	{ 0177400, 0104000, "emt", p_emt },
	{ 0177400, 0104400, "trap", p_trap },
	{ 0177777, 0000003, "bpt", p_bpt },
	{ 0177777, 0000004, "iot", p_iot },
	{ 0177777, 0000002, "rti", p_rti },
	{ 0177777, 0000006, "rtt", p_rtt },

	/* MISCELLANEOUS */

	{ 0177777, 0000000, "halt", p_halt },
	{ 0177777, 0000001, "wait", p_wait },
	{ 0177777, 0000005, "reset", p_reset },
	{ 0177777, 0000240, "nop", p_nop },

	/* CONDITION CODE OPERATORS */

	{ 0177740, 0000240, "", p_cco },

	/* ILLEGAL INSTRUCTION */

	{ 0000000, 0000000, "illegal", p_illegal },
};

static uint16_t reg[8];
static uint16_t curins;
static uint8_t quit;

/*
 * Returns 1 if src is register and 0 otherwise
 */
uint8_t parse_arg(uint16_t *adr, uint8_t arg, uint16_t incv)
{
	uint8_t mod, regn;
	uint16_t x;

	debug_print(" ");
	
	mod  = arg >> 3;
	regn = arg & 07;

	switch (mod) {
	case 0:
		debug_print("r%d", regn);

		*adr = regn;
		return 1;
	case 1:
		debug_print("(r%d)", regn);

		*adr = reg[regn];
		return 0;
	case 2:
		if (regn == 7)
			debug_print("#%o", readw(reg[regn]));
		else
			debug_print("(r%d)+", regn);

		*adr = reg[regn];
		reg[regn] += (regn >= 6) ? 2 : incv;
		return 0;
	case 3:
		*adr = readw(reg[regn]);

		if (regn == 7)
			debug_print("@#%o", *adr);
		else
			debug_print("@(r%d)+", regn);

		reg[regn] += 2;
		return 0;
	case 4:
		debug_print("-(r%d)", regn);

		reg[regn] -= (regn >= 6) ? 2 : incv;
		*adr = reg[regn];
		return 0;
	case 5:
		debug_print("@-(r%d)", regn);

		reg[regn] -= 2;
		*adr = readw(reg[regn]);
		return 0;
	case 6:
		x = readw(PC);
		PC += 2;
		if (regn == 7)
			debug_print("%06o", (uint16_t)(PC + x));
		else
			debug_print("%d(r%d)", x, regn);

		*adr = reg[regn] + x;
		return 0;
	case 7:
		x = readw(PC);
		PC += 2;
		if (regn == 7)
			debug_print("@%06o", (uint16_t)(PC + x));
		else
			debug_print("@%d(r%d)", x, regn);

		*adr = readw(reg[regn] + x);
		return 0;
	default:
		system_exit(SYSTEM_ERROR,
			    "error: mod cannot be larger than 7\n");
		return 2;
	}
}

uint8_t get_src(uint16_t *adr)
{
	uint8_t ss;
	uint8_t is_reg;

	ss = (uint8_t)((curins & 0007700) >> 6);

	is_reg = parse_arg(adr, ss, 2);
	debug_print(",");

	return is_reg;
}

uint8_t get_dst(uint16_t *adr)
{
	uint8_t dd;
	
	dd = (uint8_t)(curins & 0000077);

	return parse_arg(adr, dd, 2);
}

uint8_t get_srcb(uint16_t *adr)
{
	uint8_t ss;
	uint8_t is_reg;

	ss = (uint8_t)((curins & 0007700) >> 6);

	is_reg = parse_arg(adr, ss, 1);
	debug_print(",");

	return is_reg;
}

uint8_t get_dstb(uint16_t *adr)
{
	uint8_t dd;
	
	dd = (uint8_t)(curins & 0000077);

	return parse_arg(adr, dd, 1);
}

void p_clr(void)
{
	uint16_t dst_adr;

	if (get_dst(&dst_adr)) {
		reg[dst_adr] = 0;
	} else {
		writew(dst_adr, 0);
	}

	flag->N = flag->V = flag->C = 0;
	flag->Z = 1;
}

void p_clrb(void)
{
	uint16_t dst_adr;

	if (get_dstb(&dst_adr)) {
		((uint8_t *)&reg[dst_adr])[0] = 0;
	} else {
		writeb(dst_adr, 0);
	}

	flag->N = flag->V = flag->C = 0;
	flag->Z = 1;
}

void p_com(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] = ~reg[dst_adr];
	} else {
		val = ~readw(dst_adr);
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
	flag->C = 1;
}

void p_comb(void)
{
	uint16_t dst_adr;
	uint8_t val;
	
	if (get_dstb(&dst_adr)) {
		val = ~((uint8_t *)&reg[dst_adr])[0];
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		val = ~readb(dst_adr);
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
	flag->C = 1;
}

void p_inc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] += 1;
	} else {
		val = readw(dst_adr) + 1;
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0100000);
}

void p_incb(void)
{
	uint16_t dst_adr;
	uint8_t val;

	if (get_dst(&dst_adr)) {
		val = ((uint8_t *)&reg[dst_adr])[0] += 1;
	} else {
		val = readb(dst_adr) + 1;
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0200);
}

void p_dec(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] -= 1;
	} else {
		val = readw(dst_adr) - 1;
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0077777);
}

void p_decb(void)
{
	uint16_t dst_adr;
	uint8_t val;
	
	if (get_dstb(&dst_adr)) {
		val = ((uint8_t *)&reg[dst_adr])[0] -= 1;
	} else {
		val = readb(dst_adr) - 1;
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0177);
}

void p_neg(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		reg[dst_adr] = val = ~reg[dst_adr] + 1;
	} else {
		val = ~readw(dst_adr) + 1;
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0100000);
	flag->C = !flag->Z;
}

void p_negb(void)
{
	uint16_t dst_adr;
	uint8_t val;

	if (get_dstb(&dst_adr)) {
		val = ~((uint8_t *)&reg[dst_adr])[0] + 1;
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		val = ~readb(dst_adr) + 1;
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0200);
	flag->C = !flag->Z;
}

void p_tst(void)
{
	uint16_t dst_adr;
	uint16_t val;
	
	val = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = flag->C = 0;
}

void p_tstb(void)
{
	uint16_t dst_adr;
	uint8_t val;
	
	val = (get_dstb(&dst_adr)) ? (uint8_t)reg[dst_adr] : readb(dst_adr);

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = flag->C = 0;
}

void p_ror(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint16_t val;
	uint16_t new_c;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	
	new_c = val & 01;
	val >>= 1;
	if (flag->C) {
		val |= 0100000;
	}

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (new_c != flag->C);
	flag->C = new_c;
}

void p_rorb(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint8_t val;
	uint8_t new_c;

	val = (dst_is_reg = get_dstb(&dst_adr))
		? (uint8_t)reg[dst_adr] : readb(dst_adr);

	
	new_c = val & 01;
	val >>= 1;
	if (flag->C) {
		val |= 0200;
	}

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (new_c != flag->C);
	flag->C = new_c;
}

void p_rol(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint16_t val;
	uint16_t new_c;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	new_c = (val & 0100000) >> 15;
	val <<= 1;
	if (flag->C) {
		val |= 01;
	}

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (new_c != flag->C);
	flag->C = new_c;
}

void p_rolb(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint8_t val;
	uint8_t new_c;

	val = (dst_is_reg = get_dstb(&dst_adr))
		? (uint8_t)reg[dst_adr] : readb(dst_adr);

	new_c = (val & 0200) >> 7;
	val <<= 1;
	if (flag->C) {
		val |= 01;
	}

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (new_c != flag->C);
	flag->C = new_c;
}

void p_asr(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
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

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = flag->N ^ new_c;
	flag->C = new_c;
}

void p_asrb(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint8_t new_c;
	int8_t val;

	val = (dst_is_reg = get_dst(&dst_adr))
		? (int8_t)reg[dst_adr] : readb(dst_adr);

	new_c = val & 01;
	val >>= 1;

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = flag->N ^ new_c;
	flag->C = new_c;
}

void p_asl(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
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

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = flag->N ^ new_c;
	flag->C = new_c;
}

void p_aslb(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint16_t new_c;
	int8_t val;

	val = (dst_is_reg = get_dst(&dst_adr))
		? (int8_t)reg[dst_adr] : readb(dst_adr);

	new_c = (val & 0200) >> 7;
	val <<= 1;

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = flag->N ^ new_c;
	flag->C = new_c;
}

void p_swab(void)
{
	uint16_t dst_adr;
	uint8_t dst_is_reg;
	uint16_t val;

	val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	((uint8_t *)&val)[0] ^= ((uint8_t *)&val)[1];
	((uint8_t *)&val)[1] ^= ((uint8_t *)&val)[0];
	((uint8_t *)&val)[0] ^= ((uint8_t *)&val)[1];

	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->N = ((((uint8_t *)&val)[1] & 0200) != 0);
	flag->Z = (((uint8_t *)&val)[1] == 0);
	flag->V = flag->C = 0;
}

void p_adc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] += flag->C;
	} else {
		val = readw(dst_adr) + flag->C;
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0100000 && flag->C);
	flag->C = (flag->Z && flag->C);
}

void p_adcb(void)
{
	uint16_t dst_adr;
	uint8_t val;

	if (get_dstb(&dst_adr)) {
		((uint8_t *)&reg[dst_adr])[0] += flag->C;
		val = ((uint8_t *)&reg[dst_adr])[0];
	} else {
		val = readb(dst_adr) + flag->C;
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0200 && flag->C);
	flag->C = (flag->Z && flag->C);
}

void p_sbc(void)
{
	uint16_t dst_adr;
	uint16_t val;

	if (get_dst(&dst_adr)) {
		val = reg[dst_adr] -= flag->C;
	} else {
		val = readw(dst_adr) - flag->C;
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0077777 && flag->C);
	flag->C = (val == 0177777 && flag->C);
}

void p_sbcb(void)
{
	uint16_t dst_adr;
	uint8_t val;

	if (get_dstb(&dst_adr)) {
		((uint8_t *)&reg[dst_adr])[0] -= flag->C;
		val = ((uint8_t *)&reg[dst_adr])[0];
	} else {
		val = readb(dst_adr) - flag->C;
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = (val == 0177 && flag->C);
	flag->C = (val == 0377 && flag->C);
}

void p_sxt(void)
{
	uint16_t dst_adr;
	uint16_t val;

	val = (flag->N) ? 0177777 : 0;
	if (get_dst(&dst_adr)) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->Z = (val == 0);
}

void p_halt(void)
{
	quit = 1;
}

void p_mov(void)
{
	uint16_t src_adr, dst_adr, val;

	val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);

	if (get_dst(&dst_adr)) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_movb(void)
{
	uint16_t src_adr, dst_adr;
	uint8_t val;

	val = (get_srcb(&src_adr)) ? (uint8_t)reg[src_adr] : readb(src_adr);

	if (get_dstb(&dst_adr)) {
		reg[dst_adr] = (int8_t)val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_cmp(void)
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

	flag->N = (neg_bit != 0);
	flag->Z = (val == 0);
	flag->V = (src_neg_bit != dst_neg_bit)
		&& (dst_neg_bit == neg_bit);
	flag->C = (dst_val > src_val);
}

void p_cmpb(void)
{
	uint16_t src_adr, dst_adr;
	uint8_t src_val, dst_val, val;
	uint8_t neg_bit, src_neg_bit, dst_neg_bit;

	src_val = (get_srcb(&src_adr)) ? (uint8_t)reg[src_adr] : readb(src_adr);
	dst_val = (get_dstb(&dst_adr)) ? (uint8_t)reg[dst_adr] : readb(dst_adr);

	val = src_val - dst_val;

	neg_bit = val & 0200;
	src_neg_bit = src_val & 0200;
	dst_neg_bit = dst_val & 0200;

	flag->N = (neg_bit != 0);
	flag->Z = (val == 0);
	flag->V = (src_neg_bit != dst_neg_bit)
		&& (dst_neg_bit == neg_bit);
	flag->C = (dst_val > src_val);
}

void p_add(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val;
	uint8_t dst_is_reg;
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

	flag->N = (neg_bit != 0);
	flag->Z = (val == 0);
	flag->V = (src_neg_bit == dst_neg_bit)
		&& (dst_neg_bit != neg_bit);
	flag->C = (val < dst_val);
}

void p_sub(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val;
	uint8_t dst_is_reg;
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

	flag->N = (neg_bit != 0);
	flag->Z = (val == 0);
	flag->V = (src_neg_bit != dst_neg_bit)
		&& (src_neg_bit == neg_bit);
	flag->C = (src_val > dst_val);
}

void p_bit(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;

	src_val = (get_src(&src_adr)) ? reg[src_adr] : readw(src_adr);
	dst_val = (get_dst(&dst_adr)) ? reg[dst_adr] : readw(dst_adr);

	val = src_val & dst_val;

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_bitb(void)
{
	uint16_t src_adr, dst_adr;
	uint8_t src_val, dst_val, val;

	src_val = (get_srcb(&src_adr)) ? (uint8_t)reg[src_adr] : readb(src_adr);
	dst_val = (get_dstb(&dst_adr)) ? (uint8_t)reg[dst_adr] : readb(dst_adr);

	val = src_val & dst_val;

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_bic(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val;
	uint8_t dst_is_reg;
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

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_bicb(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;
	uint8_t dst_is_reg;

	src_val = (get_srcb(&src_adr)) ? (uint8_t)reg[src_adr] : readb(src_adr);
	dst_val = (dst_is_reg = get_dstb(&dst_adr))
		? (uint8_t)reg[dst_adr] : readb(dst_adr);

	val = ~src_val & dst_val;

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_bis(void)
{
	uint16_t src_adr, src_val;
	uint16_t dst_adr, dst_val;
	uint8_t dst_is_reg;
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

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_bisb(void)
{
	uint16_t src_adr, dst_adr;
	uint16_t src_val, dst_val, val;
	uint8_t dst_is_reg;

	src_val = (get_srcb(&src_adr)) ? (uint8_t)reg[src_adr] : readb(src_adr);
	dst_val = (dst_is_reg = get_dstb(&dst_adr))
		? (uint8_t)reg[dst_adr] : readb(dst_adr);

	val = src_val | dst_val;

	if (dst_is_reg) {
		((uint8_t *)&reg[dst_adr])[0] = val;
	} else {
		writeb(dst_adr, val);
	}

	flag->N = ((val & 0200) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_mul(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint32_t val;
	uint8_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);
	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);

	val = (uint32_t)reg[r] * src_val;
	
	if (r % 2) {
		reg[r] = val;

		flag->Z = (reg[r] == 0);
		flag->C = (val != reg[r]);
	} else {
		*(uint32_t *)&reg[r] = val;

		flag->Z = (val == 0);
		flag->C = (val != reg[r+1]);
	}

	flag->N = ((reg[r] & 0100000) != 0);
	flag->V = 0;
}

void p_div(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint16_t val;
	uint16_t rem;
	uint8_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);
	if (r % 2)
		p_illegal();
	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);

	flag->V = flag->C = 1;
	if (src_val != 0) {
		val = *(uint32_t *)&reg[r] / src_val;
		rem = *(uint32_t *)&reg[r] % src_val;

		if (abs(reg[r]) <= abs(src_val)) {
			flag->V = 0;
		}

		reg[r] = val;
		reg[r+1] = rem;

		flag->C = 0;
	}
	
	flag->N = ((reg[r] & 0100000) != 0);
	flag->Z = (reg[r] == 0);
}

void p_ash(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint16_t neg_bit, old_neg_bit;
	uint8_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);
	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);

	old_neg_bit = reg[r] & 0100000;

	if (src_val & 040) {
		src_val = ~src_val;
		src_val &= 077;

		reg[r] >>= src_val;
		flag->C = reg[r] & 01;
		reg[r] >>= 1;
	} else {
		src_val &= 077;

		reg[r] <<= src_val - 1;
		flag->C = ((reg[r] & 0100000) != 0);
		reg[r] <<= 1;
	}

	neg_bit = reg[r] & 0100000;

	flag->N = (neg_bit != 0);
	flag->Z = (reg[r] == 0);
	flag->V = (neg_bit != old_neg_bit);
}

void p_ashc(void)
{
	uint16_t src_adr;
	uint16_t src_val;
	uint16_t neg_bit, old_neg_bit;
	uint32_t val;
	uint8_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);
	if (r == 7) {
		p_illegal();
	}
	src_val = (get_dst(&src_adr)) ? reg[src_adr] : readw(src_adr);

	val = *(uint32_t *)&reg[r];
	old_neg_bit = val & 020000000000;

	if (src_val & 040) {
		src_val = ~src_val;
		src_val &= 077;

		val >>= src_val;
		flag->C = val & 01;
		val >>= 1;
	} else {
		src_val &= 077;

		val <<= src_val - 1;
		flag->C = ((val & 020000000000) != 0);
		val <<= 1;
	}
	*(uint32_t *)&reg[r] = val;

	neg_bit = val & 020000000000;

	flag->N = (neg_bit != 0);
	flag->Z = (val == 0);
	flag->V = (neg_bit != old_neg_bit);
}

void p_xor(void)
{
	uint16_t dst_adr;
	uint16_t dst_val;
	uint8_t dst_is_reg;
	uint16_t val;
	uint8_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);
	dst_val = (dst_is_reg = get_dst(&dst_adr))
		? reg[dst_adr] : readw(dst_adr);

	val = reg[r] ^ dst_val;
	if (dst_is_reg) {
		reg[dst_adr] = val;
	} else {
		writew(dst_adr, val);
	}

	flag->N = ((val & 0100000) != 0);
	flag->Z = (val == 0);
	flag->V = 0;
}

void p_br_if(uint8_t b)
{
	uint16_t offset;

	offset = curins & 0377;

	if (offset & 0200) {
		offset = ~offset;
		offset += 1;
		offset &= 0377;
		offset = -offset;
	}

	offset *= 2;
	
	debug_print(" .%d", (int16_t)offset + 2);

	if (b)
		PC += offset;
}

void p_br(void)
{
	p_br_if(1);
}

void p_bne(void)
{
	p_br_if(!flag->Z);
}

void p_beq(void)
{
	p_br_if(flag->Z);
}

void p_bpl(void)
{
	p_br_if(!flag->N);
}

void p_bmi(void)
{
	p_br_if(flag->N);
}

void p_bvc(void)
{
	p_br_if(!flag->V);
}

void p_bvs(void)
{
	p_br_if(flag->V);
}

void p_bcc(void)
{
	p_br_if(!flag->C);
}

void p_bcs(void)
{
	p_br_if(flag->C);
}

void p_bge(void)
{
	p_br_if((flag->N ^ flag->V) == 0);
}

void p_blt(void)
{
	p_br_if((flag->N ^ flag->V) == 1);
}

void p_bgt(void)
{
	p_br_if((flag->Z | (flag->N ^ flag->V)) == 0);
}

void p_ble(void)
{
	p_br_if((flag->Z | (flag->N ^ flag->V)) == 1);
}

void p_bhi(void)
{
	p_br_if((flag->C | flag->Z) == 0);
}

void p_blo(void)
{
	p_br_if((flag->C | flag->Z) == 1);
}

void p_jmp(void)
{
	uint16_t dst_adr;

	if (get_dst(&dst_adr)) {
		p_illegal();
	} else {
		PC = dst_adr;
	}
}

void p_jsr(void)
{
	uint16_t dst_adr;
	uint16_t r;

	r = (curins & 0000700) >> 6;
	debug_print(" r%d", r);

	writew(SP -= 2, reg[r]);
	reg[r] = PC;
	if (get_dst(&dst_adr)) {
		p_illegal();
	} else {
		PC = dst_adr;
	}
}

void p_rts(void)
{
	uint16_t r;

	r = curins & 07;
	debug_print(" r%d", r);

	PC = reg[r];
	reg[r] = readw(SP);
	SP +=2;
}

void p_mark(void)
{
	uint16_t nn;

	nn = curins & 077;

	SP += 2 * nn;
	PC = reg[5];
	reg[5] = readw(SP);
	SP += 2;
}

void p_sob(void)
{
	uint16_t r, nn;

	nn = (curins & 0000077) * 2;
	r = (curins & 0000700) >> 6;

	debug_print(" r%d .-%d", r, nn - 2);

	--reg[r];

	if (reg[r])
		PC -= nn;
}

void p_emt(void)
{
	writew(SP -= 2, preadw(A_PSW));
	writew(SP -= 2, PC);

	PC = preadw(030);
	pwritew(A_PSW, preadw(032));
}

void p_trap(void)
{
	writew(SP -= 2, preadw(A_PSW));
	writew(SP -= 2, PC);

	PC = preadw(034);
	pwritew(A_PSW, preadw(036));
}

void p_bpt(void)
{
	writew(SP -= 2, preadw(A_PSW));
	writew(SP -= 2, PC);

	PC = preadw(014);
	pwritew(A_PSW, preadw(016));
}

void p_iot(void)
{
	writew(SP -= 2, preadw(A_PSW));
	writew(SP -= 2, PC);

	PC = preadw(020);
	pwritew(A_PSW, preadw(022));
}

void p_rti(void)
{
	PC = readw(SP);
	pwritew(A_PSW, readw(SP += 2));
	SP += 2;
}

void p_rtt(void)
{
	PC = readw(SP);
	pwritew(A_PSW, readw(SP += 2));
	SP += 2;
}

void p_wait(void)
{
	PC -= 2;
}

void p_reset(void)
{
	unibus_init();
}

void p_nop(void)
{
}

void p_cco(void)
{
	uint16_t xx;
	uint8_t setv;

	xx = curins & 037;

	setv = ((xx & 020) != 0);

	if (setv)
		debug_print("set flags: ");
	else
		debug_print("clear flags: ");

	if (xx & 010) {
		flag->N = setv;
		debug_print("N");
	}
	if (xx & 004) {
		flag->Z = setv;
		debug_print("Z");
	}
	if (xx & 002) {
		flag->V = setv;
		debug_print("V");
	}
	if (xx & 001) {
		flag->C = setv;
		debug_print("C");
	}
}

void p_illegal(void)
{
	system_exit(SYSTEM_ERROR, "error: illegal instruction\n");
}

void pdp11_int(uint32_t pc)
{
	writew(SP -= 2, preadw(A_PSW));
	writew(SP -= 2, PC);

	PC = preadw(pc);
	pwritew(A_PSW, preadw(pc + 2));
}

void pdp11_run(void)
{
	uint8_t i, n;
	uint16_t xbuf;
	int rbuf;
	
	PC = 01000;

	unibus_init();
	flag = get_low_psw();

	for (;;) {
		//mmu_next_ispace();
		curins = readw(PC);
		debug_print_regs(reg);
		debug_print_init();
		debug_print("%06o %06o: ", PC, curins);
		PC += 2;

		for (i = 0, n = sizeof(ins)/sizeof(instruction_t);
		     i < n;
		     ++i) {
			if ((curins & ins[i].mask) == ins[i].opcode) {
				debug_print(ins[i].name);
				ins[i].exec();
				debug_refresh();
				break;
			}
		}

		if ((xbuf = preadw(A_XBUF)) != 0) {
			terminal_putchar(xbuf);
			pwritew(A_XBUF, 0);
		}

		if ((rbuf = terminal_getchar()) != TERMINAL_NOCH) {
			pwritew(A_RBUF, rbuf);
			pwritew(A_RCSR, 0000200);
		}

		if (rbuf_readed()) {
			pwritew(A_RCSR, 0000000);
		}

		rk11_cycle();

		if (quit) {
			terminal_refresh();
			terminal_getch();
			break;
		}
	}
}
