#include "kw11l.h"
#include "memory.h"
#include "processor.h"

#include <time.h>

#define KW11L_INT_EN  0000100
#define KW11L_MONITOR 0000200

static clock_t clk;

void kw11l_init(void)
{
	pwritew(A_LKS, 0000000);

	clk = clock();
}

void kw11l_cycle(void)
{
	uint16_t lks;

	lks = preadw(A_LKS);

	if (lks & KW11L_MONITOR == 0
	    && (clock() - clk) * 1000 / CLOCKS_PER_SEC >= 20) {
		lks |= KW11L_MONITOR;
		pwritew(A_LKS, lks);
		clk = clock();

		if (lks & KW11L_INT_EN)
			pdp11_int(0100, 6);
	}
}
