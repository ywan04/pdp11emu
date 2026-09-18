#ifndef MMU__H
#define MMU__H

#include <stdint.h>

uint16_t mmu_enabled(void);
uint32_t mmu_get_physical(uint16_t vadr);
void mmu_use_ispace(void);
void mmu_cycle(void);
void mmu_preinit(void);

#endif /* MMU__H */
