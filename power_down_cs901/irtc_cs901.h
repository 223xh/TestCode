#ifndef I_RTC_H
#define I_RTC_H

#define RTC_COUNTER_ADDR        0
#define RTC_GPO_COUNTER_ADDR    1					
#define RTC_SEC_ADJUST_ADDR     2	
#define RTC_REGMEM_ADDR_0       3
#define RTC_REGMEM_ADDR_1       4

#define RTC_ADDR0               0x01200280 //RTC register  RTC_ADDR0
#define RTC_ADDR1               0x01200284
#define RTC_ADDR2               0x01200288
#define RTC_ADDR3               0x0120028C

#define     RTC_REG0                (volatile unsigned long *)RTC_ADDR0
#define     RTC_REG1                (volatile unsigned long *)RTC_ADDR1
#define     RTC_REG2                (volatile unsigned long *)RTC_ADDR2
#define     RTC_REG3		            (volatile unsigned long *)RTC_ADDR3


#define     C_TIMER_MUX             (volatile unsigned long *)0x01000530
#define     C_TIMERE                (volatile unsigned long *)0x01000544


//register0's bits
#define test_bypass             0x08
#define sdi                     0x04
#define sen                     0x02
#define sclk                    0x01
#define sbus                    (sen|sclk|sdi)

//register1's bits
#define s_ready                 0x02
#define Sdo                     0x01

//register3's bits
#define COUNT_ALWAYS   1 << 17

#define RTC_sbus_LOW(x)            ((*(volatile unsigned *)RTC_ADDR0) &= ~sbus)
#define RTC_sdi_HIGH(x)            ((*(volatile unsigned *)RTC_ADDR0) |=  sdi )
#define RTC_sdi_LOW(x)             ((*(volatile unsigned *)RTC_ADDR0) &= ~sdi )
#define RTC_sen_HIGH(x)            ((*(volatile unsigned *)RTC_ADDR0) |=  sen )
#define RTC_sen_LOW(x)             ((*(volatile unsigned *)RTC_ADDR0) &= ~sen )
#define RTC_sclk_HIGH(x)           ((*(volatile unsigned *)RTC_ADDR0) |=  sclk)
#define RTC_sclk_LOW(x)            ((*(volatile unsigned *)RTC_ADDR0) &= ~sclk)
#define RTC_Sdo_READBIT            ((*(volatile unsigned *)RTC_ADDR1)&Sdo)
#define RTC_CountAlways_HIGH(x) ((*(volatile unsigned *)RTC_ADDR3) |= COUNT_ALWAYS)
#define RTC_CountAlways_LOW(x)  ((*(volatile unsigned *)RTC_ADDR3) &= ~COUNT_ALWAYS)

#define RTC_COUNTER_BITS        32
#define RTC_GPO_COUNTER_BITS    22
#define RTC_SER_ADDR_BITS       3
#define RTC_SER_DATA_BITS       4
#define RTC_SER_BYTE_BITS       3
#define RTC_USE_REGMEM          2*32
#define RTC_REGMEM_NUM          2

#define     set_rtc_TEST_BYPASS(a)  ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 3)) | (a << 3)))
#define     set_rtc_TEST_CLK(a)     ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 4)) | (a << 4)))
#define     set_rtc_TEST_MODE(a)    ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 5)) | (a << 5)))

#endif