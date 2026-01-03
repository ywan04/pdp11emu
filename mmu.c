#include "mmu.h"
#include "memory.h"
#include "processor.h"
#include "system.h"

enum {
	MMU_REG_USER,
	MMU_REG_SUPERVISOR,
	MMU_REG_KERNEL
};

struct {
	uint16_t par[2][8];
	uint16_t pdr[2][8];
} *reg[3];

uint8_t cur_space;

uint16_t mmu_enabled(void)
{
	uint16_t sr0_data;

	sr0_data = preadw(A_SR0);

	return sr0_data & 01;
}

uint32_t mmu_get_physical(uint16_t vadr)
{
	uint32_t adr;
	uint8_t regi;

	switch(psw_get_mode()) {
	case PSW_MODE_USER:
		regi = MMU_REG_USER;
		break;
	case PSW_MODE_SUPERVISOR:
		regi= MMU_REG_SUPERVISOR;
		break;
	case PSW_MODE_KERNEL:
		regi = MMU_REG_KERNEL;
		break;
	default:
		system_exit(SYSTEM_ERROR, "error: MMU invalid mode\n");
	}

	adr = reg[regi]->par[cur_space][vadr >> 13];
	adr <<= 6;
	vadr &= 017777;
	adr += vadr;

	return adr;
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
	reg[MMU_REG_USER]       = mem_get_user_regs();
	reg[MMU_REG_SUPERVISOR] = mem_get_supervisor_regs();
	reg[MMU_REG_KERNEL]     = mem_get_kernel_regs();
}
