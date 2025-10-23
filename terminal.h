#ifndef TERMINAL__H
#define TERMINAL__H

#define TERMINAL_Y 0
#define TERMINAL_X (COLS/2)

#define TERMINAL_HEIGHT LINES
#define TERMINAL_WIDTH (COLS/2)

void terminal_create(void);
void terminal_destroy(void);
void terminal_system_destroy(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_getch(void);
void terminal_refresh(void);

#endif /* TERMINAL__H */
