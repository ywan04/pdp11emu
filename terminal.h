#ifndef TERMINAL__H
#define TERMINAL__H

#include <ncurses.h>

#define TERMINAL_Y 0
#define TERMINAL_X (COLS/2)

#define TERMINAL_HEIGHT LINES
#define TERMINAL_WIDTH (COLS/2)

#define TERMINAL_NOCH ERR

void terminal_create(void);
void terminal_destroy(void);
void terminal_system_destroy(void);
void terminal_clear(void);
void terminal_putchar(char c);
char terminal_getchar(void);
void terminal_getch(void);
void terminal_refresh(void);

#endif /* TERMINAL__H */
