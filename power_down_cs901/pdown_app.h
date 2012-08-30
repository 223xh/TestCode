#ifndef P_DOWN_APP_H
#define P_DOWN_APP_H

#include <includes.h>
#include "irtc_cs901.h"

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

extern unsigned irtc_read(unsigned addr);
extern int irtc_write(unsigned addr, unsigned data);

#endif