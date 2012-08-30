#include "includes.h"
#include "aw_windows.h"

#define C_TIMER_MUX (*(volatile unsigned *)0x01000530)
#define C_TIMER_E   (*(volatile unsigned *)0x01000544)

#define RTC_COUNTER_ADDR       0

#define WRITE_BACK_INTERVAL    60

static unsigned rtc_start;
static double rtc_devt = 0.0, rtc_adj = 1.0;
static unsigned rtc_soft_counter = 0, rtc_soft_counter_hi = 0;
static unsigned last_timer = 0;
static unsigned wr_back_interval = WRITE_BACK_INTERVAL;

static void set_timer_e_clock(unsigned sel)
{
    unsigned val;

    sel = sel>4?4:sel;
    val = C_TIMER_MUX;
    val &= ~(7<<8);
    val |= (sel<<8);
    C_TIMER_MUX = val;
}

void rtc_deviation_set(unsigned devt)
{
    rtc_devt = (double)(12000000.0/(double)devt);
}

void rtc_adjust(unsigned adj, unsigned char dirct)  //0,slow.1,fast
{
    if(!adj){
        rtc_adj = 1.0;
        return;
    }

    if(dirct)
        rtc_adj = (double)(1.0 + (1.0/(double)adj));
    else
        rtc_adj = (double)(1.0 - (1.0/(double)adj));
}

unsigned rtc_soft_get()
{
    return rtc_soft_counter;
}

void rtc_soft_adjust_init()
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    set_timer_e_clock(4);//set TIMER_E to 1ms per num
    rtc_start = ser_access_read(RTC_COUNTER_ADDR);
    //while(ser_access_read(RTC_COUNTER_ADDR) != (rtc_start + 1));
    while(ser_access_read(RTC_COUNTER_ADDR) == rtc_start);
    rtc_start += 1;
    C_TIMER_E = 0;
    last_timer = 0;
    rtc_soft_counter = rtc_start;
    rtc_soft_counter_hi = 0;
    wr_back_interval = WRITE_BACK_INTERVAL;
    //rtc_adj = 1.0;
    OS_EXIT_CRITICAL();
}

void rtc_soft_adjust_irq()
{
    unsigned cur_timer = 0;
    OS_CPU_SR  cpu_sr;
    unsigned remainder;
    
    cur_timer = C_TIMER_E;
    
    if(cur_timer < last_timer){
        rtc_soft_counter_hi += 1;
    }
        


    rtc_soft_counter = (unsigned)((double) rtc_start + ((double)cur_timer*rtc_devt*rtc_adj/1000.0) + ((double)rtc_soft_counter_hi * 4294967.296));
    //remainder = ((unsigned)(((double)cur_timer*rtc_devt*rtc_adj) + ((double)rtc_soft_counter_hi * 296.0)))%1000;

    if(!(wr_back_interval--))
    {
        wr_back_interval = WRITE_BACK_INTERVAL;
        
        do{
            cur_timer = C_TIMER_E;
            rtc_soft_counter = (unsigned)((double) rtc_start + ((double)cur_timer*rtc_devt*rtc_adj/1000.0) + ((double)rtc_soft_counter_hi * 4294967.296));
            remainder = ((unsigned)(((double)cur_timer*rtc_devt*rtc_adj) + ((double)rtc_soft_counter_hi * 296.0)))%1000;
            AVTimeDly(2);
        }while(remainder<990)
        
        OS_ENTER_CRITICAL();
        ser_access_write(RTC_COUNTER_ADDR, rtc_soft_counter + 1);
        OS_EXIT_CRITICAL();
    }

    last_timer = cur_timer;
}