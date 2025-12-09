#include "dl11.h"
#include "memory.h"
#include "terminal.h"

static char rbuf_r;

char rbuf_readed(void)
{
	if (rbuf_r) {
		rbuf_r = 0;
		return 1;
	}

	return 0;
}

void dl11_init(void)
{
	pwritew(A_XCSR, 0000200); /* XCSR: transmitter ready */
	pwritew(A_XBUF, 0000000);
	pwritew(A_RCSR, 0000000);
	pwritew(A_RBUF, 0000000);
}

void dl11_cycle(void)
{
	uint16_t xbuf;
	int rbuf;

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
}

void dl11_addressing(uint32_t padr)
{
	if (padr == A_RBUF)
		rbuf_r = 1;
}
