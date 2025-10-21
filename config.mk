VERSION = 0.1

# flags
#CFLAGS  = -std=c99 -pedantic -Wall -O2 -DVERSION=\"${VERSION}\"
CFLAGS  = -g -std=c99 -pedantic -Wall -O0 -DVERSION=\"${VERSION}\" -DDEBUG_ENABLE
LDFLAGS = -lncurses

# compiler and linker
CC = cc
