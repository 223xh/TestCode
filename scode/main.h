#ifndef _MAIN_H
#define _MAIN_H

#include <stdio.h>
#include <string.h>

#define REG(a) (*(volatile unsigned*)(a))

#ifdef AML_NIKED3
#define HW_WATCHDOG 0x01000448
#define C_TIMER_MUX (*(volatile unsigned *)0x01000530)
#define C_TIMER_E (*(volatile unsigned *)0x01000544)

#define MPEG_PLL_CNTL   0x01100568
#define MPEG_CLOCK_CNTL 0x01100574

#define DRAM_CTRL_REG1  0x01101c00
#define DRAM_MASK_0     0x01101d10
#define DRAM_MASK_1     0x01101d14

#define PERIPHS_PIN_MUX_4 0x012000c0
#define PERIPHS_PIN_MUX_3 0x012000bc
#define PERIPHS_PIN_MUX_2 0x012000b8
#define PERIPHS_PIN_MUX_1 0x012000b4
#define PERIPHS_PIN_MUX_0 0x012000b0
#else

#endif

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

#define DISABLE_WATCHDOG() (REG(HW_WATCHDOG) = 0)

void delay_ms(unsigned ms);

#endif