#include "pdown_app.h"

#define PDOWN_FLAG 2009

void run_pdown_app(void)
{
    irtc_write(RTC_REGMEM_ADDR_1, PDOWN_FLAG);
    wr(0x1000448, 0x400001);
}

void set_wakeup_time(unsigned sec)
{
    irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
    irtc_write(RTC_GPO_COUNTER_ADDR, (0x600000 | (sec & 0xfffff)));//(0-19bit seconds)
}