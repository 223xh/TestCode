#include "includes.h"
#include "aw_windows.h"

#define C_TIMER_MUX (*(volatile unsigned *)0x01000530)
#define C_TIMER_E (*(volatile unsigned *)0x01000544)

#define RTC_COUNTER_ADDR            0

#define RTC_ADDR0                   (*(volatile unsigned long *)(0x01200280))
#define RTC_ADDR1                   (*(volatile unsigned long *)(0x01200284))
#define RTC_ADDR2                   (*(volatile unsigned long *)(0x01200288))
#define RTC_ADDR3                   (*(volatile unsigned long *)(0x0120028c))
#define RTC_ADDR4                   (*(volatile unsigned long *)(0x01200290))

static void m_delay(unsigned us)
{
    C_TIMER_E = 0;
    while(C_TIMER_E < us);
}

static void set_timer_e_clock(unsigned sel)
{
    unsigned val;

    sel = sel>4?4:sel;
    val = C_TIMER_MUX;
    val &= ~(7<<8);
    val |= (sel<<8);
    C_TIMER_MUX = val;
}

void test_func(void)
{
    OS_CPU_SR  cpu_sr;
    unsigned i, s[2], e[2], tmp, tmp1;
    unsigned data[10][2];
    
    set_timer_e_clock(4);

    printf("start 1st test, plz wait 10 seconds...\n");
    
    RTC_ADDR3 |= (1<<17);
    
    for(i=0;i<10;i++)
    {
        data[i][0] = AVTimeGet();
        data[i][1] = RTC_ADDR2;
        m_delay(1000);
    }
    
    for(i=0;i<10;i++)
    {
        printf("[%u times] %u, %u\n", i+1, data[i][0], data[i][1]);
    }
    
    printf("start 2nd test, plz wait 10 seconds...\n");
    s[0] = AVTimeGet();
    s[1] = RTC_ADDR2;
    OS_ENTER_CRITICAL();
    m_delay(10000);
    OS_EXIT_CRITICAL();
    e[0] = AVTimeGet();
    e[1] = RTC_ADDR2;
    printf("%u, %u, %u\n",s[0],e[0],e[0]-s[0]);
    printf("%u, %u, %u\n",s[1],e[1],e[1]-s[1]);
    
    printf("start 3rd test...\n");
    
    set_timer_e_clock(1);
    
    for(i=0;i<10;i++){
    ser_access_write(RTC_COUNTER_ADDR,0);
    C_TIMER_E = 0;
    while(ser_access_read(RTC_COUNTER_ADDR)!=1);
    tmp = C_TIMER_E;
    printf("[%u times] %u\n", i+1, tmp);
    }
    
    printf("start 4th test...\n");
    s[0] = AVTimeGet();
    s[1] = ser_access_read(RTC_COUNTER_ADDR);
    for(i=0;i<10;i++){
        tmp1 = ser_access_read(RTC_COUNTER_ADDR);
        while(ser_access_read(RTC_COUNTER_ADDR)!=(tmp1 + 1));
        ser_access_write(RTC_COUNTER_ADDR,(tmp1+1));
        m_delay(1500000);
    }
    e[0] = AVTimeGet();
    e[1] = ser_access_read(RTC_COUNTER_ADDR);
    printf("%u, %u, %u\n",s[0],e[0],e[0]-s[0]);
    printf("%u, %u, %u\n",s[1],e[1],e[1]-s[1]);
    
    //set_timer_e_clock(1);
    
    //ser_access_read(RTC_COUNTER_ADDR);
    
}