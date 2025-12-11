#include "mmu.h"

enum {
	MMU_USER,
	MMU_SUPERVISOR,
	MMU_KERNEL
};

struct {
	uint16_t par[2][8];
	uint16_t pdr[2][8];
} *reg[3];

uint8_t mmu_enabled(void)
{
	// todo
	return 0;
}


uint32_t mmu_get_physical(uint16_t vadr)
{
	// todo
	return vadr;
}

void mmu_use_ispace(void)
{
	// todo
}

void mmu_cycle(void)
{
	// todo
}

void mmu_preinit(void)
{
	reg[MMU_USER]       = mem_get_user_regs();
	reg[MMU_SUPERVISOR] = mem_get_supervisor_regs();
	reg[MMU_KERNEL]     = mem_get_kernel_regs();
}
