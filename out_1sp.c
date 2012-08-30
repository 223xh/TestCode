#define RTC_ADDR0               0x01200280 //RTC register  RTC_ADDR0
#define RTC_ADDR1               0x01200284
#define RTC_ADDR2               0x01200288
#define RTC_ADDR3               0x0120028C

#define     RTC_REG0                (volatile unsigned long *)RTC_ADDR0
#define     RTC_REG1                (volatile unsigned long *)RTC_ADDR1
#define     RTC_REG2                (volatile unsigned long *)RTC_ADDR2
#define     RTC_REG3		        (volatile unsigned long *)RTC_ADDR3

#define OUT_EN 0x1200030
#define OUT_LEVEL 0x1200034
#define OUT_BIT (1<<25)

void OutputOneSecondPulse()
{
    
    _flag(0);
    *(volatile unsigned*)0x12000c4 &= ~((1<<20) | (1<<22));
    *(volatile unsigned*)0x12000bc &= ~(1<<17);
    *(volatile unsigned*)OUT_EN &= ~OUT_BIT;
    (*RTC_REG3)    &= ~(1 << 17);  // disable count always
    *RTC_REG2 = 0;
    (*RTC_REG3)    |= (1 << 17);   // Enable count always
    osc1 = *RTC_REG2;
    while(1)
    {
        if((*RTC_REG2) % 32768 == 0)
        {
            if((*(volatile unsigned*)OUT_LEVEL) & OUT_BIT)
                *(volatile unsigned*)OUT_LEVEL &= ~OUT_BIT;
            else
                *(volatile unsigned*)OUT_LEVEL |= OUT_BIT;
        }
    }
}