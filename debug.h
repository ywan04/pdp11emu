#ifndef DEBUG__H
#define DEBUG__H

#include <stdint.h>

#define DEBUG_Y (LINES/2)
#define DEBUG_X 0

#define DEBUG_HEIGHT (LINES/2)
#define DEBUG_WIDTH (COLS/2)

void debug_create(void);
void debug_destroy(void);
void debug_silent_destroy(void);
void debug_print_regs(uint16_t *reg);
void debug_print_init(void);
void debug_print(const char *format, ...);
void debug_refresh(void);

#endif /* DEBUG__H */
