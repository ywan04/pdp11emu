#include "pdp11_processor.h"
#include "pdp11_memory.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>

#define PC reg[7]

struct {
	unsigned char N : 1;
	unsigned char Z : 1;
	unsigned char V : 1;
	unsigned char C : 1;
} flag;

typedef struct {
	word_t mask;
	word_t opcode;
	char *name;
	void (*do_func)(void);
} instruction_t;

instruction_t ins[] = {
	{ 0177777, 0000000, "halt", do_halt },
	{ 0170000, 0010000, "mov", do_mov },
	{ 0170000, 0060000, "add", do_add },
};

static word_t reg[8];
static word_t curins;

void do_halt(void)
{
	trace("halt.\n");

	exit(0);
}

void do_mov(void)
{
}

void do_add(void)
{
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
