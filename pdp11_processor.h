#ifndef PDP11_PROCESSOR__H
#define PDP11_PROCESSOR__H

void run(void);

/*
 * === INSTRUCTIONS ===
 */

/* SINGLE OPERAND */
	
/* General */
void p_clr(void);
void p_com(void);
void p_inc(void);
void p_dec(void);
void p_neg(void);
void p_tst(void);
/* Rotate & Shift */
void p_ror(void);
void p_rol(void);
void p_asr(void);
void p_asl(void);
void p_swab(void);
/* Multiple Precision */
void p_adc(void);
void p_sbc(void);
void p_sxt(void);

/* DOUBLE OPERAND */
	
/* General */
void p_mov(void);
void p_cmp(void);
void p_add(void);
void p_sub(void);
/* Logical */
void p_bit(void);
void p_bic(void);
void p_bis(void);
/* Register */
void p_mul(void);
void p_div(void);
void p_ash(void);
void p_ashc(void);
void p_xor(void);

/* BRANCH */

/* Branches */
/* Signed Conditional Branches */
/* Unsigned Conditional Branches */

/* JUMP & SUBROUTINE */

void p_jmp(void);
void p_jsr(void);
void p_rts(void);
void p_mark(void);
void p_sob(void);

/* TRAP & INTERRUPT */

/* MISCELLANEOUS */

void p_halt(void);
void p_nop(void);

#endif // PDP11_PROCESSOR__H
