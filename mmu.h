#ifndef MMU__H
#define MMU__H

#include <stdint.h>

uint8_t mmu_enabled(void);
uint32_t mmu_get_physical(uint16_t vadr);
void mmu_use_ispace(void);

#endif /* MMU__H */
