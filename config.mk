VERSION = 0.3

# flags
CFLAGS  = -std=gnu99 -pedantic -Wall -O2 -DVERSION=\"${VERSION}\"
LDFLAGS = -lncurses

# compiler and linker
CC = gcc
