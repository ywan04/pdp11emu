#ifndef PDP11_PROCESSOR__H
#define PDP11_PROCESSOR__H

void run(void);

/*
 * === INSTRUCTIONS ===
 */

/* SINGLE OPERAND */
	
/* General */
void clr(void);
void com(void);
void inc(void);
void dec(void);
void neg(void);
void tst(void);
/* Rotate & Shift */
void ror(void);
void rol(void);
void asr(void);
void asl(void);
void swab(void);
/* Multiple Precision */
void adc(void);
void sbc(void);
void sxt(void);

/* DOUBLE OPERAND */
	
/* General */
void mov(void);
void cmp(void);
void add(void);
void sub(void);
/* Logical */
void bit(void);
void bic(void);
void bis(void);
/* Register */

/* BRANCH */

/* Branches */
/* Signed Conditional Branches */
/* Unsigned Conditional Branches */

/* JUMP & SUBROUTINE */

void jmp(void);
void jsr(void);
void rts(void);
void sob(void);

/* TRAP & INTERRUPT */

/* MISCELLANEOUS */

void halt(void);
void nop(void);

#endif // PDP11_PROCESSOR__H
