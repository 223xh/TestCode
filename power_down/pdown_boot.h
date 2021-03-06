#ifndef P_DOWN_BOOT_H
#define P_DOWN_BOOT_H

#include "irtc.h"
#include "adc.h"

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

extern unsigned irtc_read(unsigned addr);
extern int irtc_write(unsigned addr, unsigned data);
extern void static_reg_write(unsigned data);
extern void adc_init(unsigned ch, unsigned cal_mask);
extern unsigned adc_key_get(void);
extern void delay_us(unsigned long us);

#endif