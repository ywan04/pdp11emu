#include "pdp11_memory.h"
#include "pdp11_processor.h"

#include <assert.h>

void mem_test(void)
{
	byte_t a, b;
	word_t w;

	writew(0, 0x5498);
	a = readb(0);
	b = readb(1);
	assert(a == 0x98);
	assert(b == 0x54);

	writeb(1, 0x23);
	b = readb(1);
	assert(a == 0x98);
	assert(b == 0x23);

	w = readw(0);
	assert(w == 0x2398);
}

int main(int argc, char *argv[])
{
	mem_test();
	
	loadfile(argv[1]);
	run();

	return 0;
}
