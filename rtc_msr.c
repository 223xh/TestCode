#define RTC_ADDR0               0x01200280 //RTC register  RTC_ADDR0
#define RTC_ADDR1               0x01200284
#define RTC_ADDR2               0x01200288
#define RTC_ADDR3               0x0120028C

#define     RTC_REG0                (volatile unsigned long *)RTC_ADDR0
#define     RTC_REG1                (volatile unsigned long *)RTC_ADDR1
#define     RTC_REG2                (volatile unsigned long *)RTC_ADDR2
#define     RTC_REG3		        (volatile unsigned long *)RTC_ADDR3

static unsigned Measure32k(unsigned sec)//MAX65
{

    (*RTC_REG3)    &= ~(1 << 17);  // disable count always
    *RTC_REG2 = 0;
    *RTC_REG3 = 0;
    (*RTC_REG3)    = ((1 << 16) | ((sec*1000) & 0xffff));   // Enable count always

    AVTimeDly(10);
    while((*RTC_REG3)&(1<<20))
    {
        AVTimeDly(1000);
    }
    
    return (*RTC_REG2);
}