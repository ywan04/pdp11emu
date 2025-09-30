#ifndef PDP11_PROCESSOR__H
#define PDP11_PROCESSOR__H

void run(void);

/*
 * === INSTRUCTIONS ===
 */

/* SINGLE OPERAND */
	
/* General */
void do_clr(void);
void do_com(void);
void do_inc(void);
void do_dec(void);
void do_neg(void);
void do_tst(void);
/* Rotate & Shift */
void do_ror(void);
void do_rol(void);
void do_asr(void);
void do_asl(void);
/* Multiple Precision */

/* DOUBLE OPERAND */
	
/* General */
void do_mov(void);
void do_cmp(void);
void do_add(void);
void do_sub(void);
/* Logical */
/* Register */

/* BRANCH */

/* Branches */
/* Signed Conditional Branches */
/* Unsigned Conditional Branches */

/* JUMP & SUBROUTINE */

void do_jmp(void);
void do_jsr(void);
void do_rts(void);
void do_sob(void);

/* TRAP & INTERRUPT */

/* MISCELLANEOUS */

void do_halt(void);
void do_nop(void);

#endif // PDP11_PROCESSOR__H
