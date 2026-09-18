#ifndef MEMORY__H
#define MEMORY__H

#include <stdint.h>

#define MEMSIZE (256*1024) /* 256KB */

#define A_PSW  0777776

#define A_SR0  0777572
#define A_SR1  0777574
#define A_SR2  0777576
#define A_SR3  0772516

#define A_USRR 0777676
#define A_SPVR 0772276
#define A_KRNR 0772376

#define A_RCSR 0777560
#define A_RBUF 0777562
#define A_XCSR 0777564
#define A_XBUF 0777566

#define A_RKDS 0777400
#define A_RKER 0777402
#define A_RKCS 0777404
#define A_RKWC 0777406
#define A_RKBA 0777410
#define A_RKDA 0777412

#define A_LKS  0777546

void unibus_init(void);
void *mem_get_psw(void);
void *mem_get_user_regs(void);
void *mem_get_supervisor_regs(void);
void *mem_get_kernel_regs(void);
void mem_addressing(uint32_t adr);
void pwritew(uint32_t padr, uint16_t w);
uint16_t preadw(uint32_t padr);
void writeb(uint16_t adr, uint8_t b);
uint8_t readb(uint16_t adr);
void writew(uint16_t adr, uint16_t w);
uint16_t readw(uint16_t adr);
void loadfile(const char *filename);

#endif /* MEMORY__H */
