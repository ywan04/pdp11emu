#ifndef PDP11_PROCESSOR__H
#define PDP11_PROCESSOR__H

void run(void);

/*
 * === INSTRUCTIONS ===
 */

/* SINGLE OPERAND */
	
/* General */
void p_clr(void);
void p_clrb(void);
void p_com(void);
void p_comb(void);
void p_inc(void);
void p_incb(void);
void p_dec(void);
void p_decb(void);
void p_neg(void);
void p_negb(void);
void p_tst(void);
void p_tstb(void);
/* Rotate & Shift */
void p_ror(void);
void p_rorb(void);
void p_rol(void);
void p_rolb(void);
void p_asr(void);
void p_asrb(void);
void p_asl(void);
void p_aslb(void);
void p_swab(void);
/* Multiple Precision */
void p_adc(void);
void p_adcb(void);
void p_sbc(void);
void p_sbcb(void);
void p_sxt(void);

/* DOUBLE OPERAND */
	
/* General */
void p_mov(void);
void p_movb(void);
void p_cmp(void);
void p_cmpb(void);
void p_add(void);
void p_sub(void);
/* Logical */
void p_bit(void);
void p_bitb(void);
void p_bic(void);
void p_bicb(void);
void p_bis(void);
void p_bisb(void);
/* Register */
void p_mul(void);
void p_div(void);
void p_ash(void);
void p_ashc(void);
void p_xor(void);

/* BRANCH */

/* Branches */
void p_br(void);
void p_bne(void);
void p_beq(void);
void p_bpl(void);
void p_bmi(void);
void p_bvc(void);
void p_bvs(void);
void p_bcc(void);
void p_bcs(void);
/* Signed Conditional Branches */
void p_bge(void);
void p_blt(void);
void p_bgt(void);
void p_ble(void);
/* Unsigned Conditional Branches */
void p_bhi(void);
void p_blo(void);

/* JUMP & SUBROUTINE */

void p_jmp(void);
void p_jsr(void);
void p_rts(void);
void p_mark(void);
void p_sob(void);

/* TRAP & INTERRUPT */

void p_emt(void);
void p_trap(void);
void p_bpt(void);
void p_iot(void);
void p_rti(void);

/* MISCELLANEOUS */

void p_halt(void);
void p_nop(void);

/* CONDITION CODE OPERATORS */

void p_cco(void);

#endif // PDP11_PROCESSOR__H
