#include "pdp_processor.h"
#include "pdp_memory.h"

#include <stdio.h>

#define PC reg[7]

typedef struct {
	word_t mask;
	word_t opcode;
	char *name;
	void (*do_func)(void);
} instruction_t;

instruction_t ins[] = {
	{ 0777777, 0000000, "halt", do_halt },
	{ 0170000, 0010000, "mov", do_mov },
	{ 0170000, 0060000, "add", do_add },
};

static word_t reg[8];

void do_halt(void)
{
	printf("Halt.\n");

	exit(0);
}

void do_mov()
{
}

void do_add()
{
}

void do_nothing()
{
}

void run(void)
{
	PC = 01000;

	for (;;) {
		word_t w = readw(PC);
		printf("%06o %06o: ", PC, w); // from printf to trace
		PC += 2;

		if (w == 0) {
			do_halt();
		} else if ((w & 0170000) == 0010000) { // MOV: 01SSDD
			do_mov();
		}
	}
}
