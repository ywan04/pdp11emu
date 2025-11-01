VERSION = 0.1

# flags
CFLAGS  = -std=c99 -pedantic -Wall -O2 -DVERSION=\"${VERSION}\"
LDFLAGS = -lncurses

# compiler and linker
CC = cc
