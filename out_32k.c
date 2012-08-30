#define RTC_ADDR0               0x01200280 //RTC register  RTC_ADDR0
#define RTC_ADDR1               0x01200284
#define RTC_ADDR2               0x01200288
#define RTC_ADDR3               0x0120028C

#define     RTC_REG0                (volatile unsigned long *)RTC_ADDR0
#define     RTC_REG1                (volatile unsigned long *)RTC_ADDR1
#define     RTC_REG2                (volatile unsigned long *)RTC_ADDR2
#define     RTC_REG3		            (volatile unsigned long *)RTC_ADDR3

void Output32K()
{
    register unsigned osc1, osc2;
    
    _flag(0);
    *(volatile unsigned*)0x12000b0 &= ~((1<<11) | (1<<14));
    *(volatile unsigned*)0x12000c4 &= ~(1<<25);
    *(volatile unsigned*)0x1200030 &= ~(1<<4);
    (*RTC_REG3)    &= ~(1 << 17);  // disable count always
    *RTC_REG2 = 0;
    (*RTC_REG3)    |= (1 << 17);   // Enable count always
    osc1 = *RTC_REG2;
    while(1)
    {
        osc2 = *RTC_REG2;
        if(osc2 - osc1)
        {
            osc1 = osc2;
            if((*(volatile unsigned*)0x1200034) & (1<<4))
                *(volatile unsigned*)0x1200034 &= ~(1<<4);
            else
                *(volatile unsigned*)0x1200034 |= (1<<4);
        }
    }
}