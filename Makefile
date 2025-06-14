# pdp11emu - PDP-11 emulator
# See LICENSE file for copyright and license details.

include config.mk

SRC = main.c pdp11.c
OBJ = ${SRC:.c=.o}

all: pdp11emu

.c.o:
	${CC} -c ${CFLAGS} $<

pdp11emu: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f pdp11emu *.o
