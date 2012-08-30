#ifndef I_RTC_H
#define I_RTC_H

#define RTC_COUNTER_ADDR        0
#define RTC_GPO_COUNTER_ADDR    1
#define RTC_SEC_ADJUST_ADDR     2
#define RTC_UNUSED_ADDR_1       3
#define RTC_REGMEM_ADDR_0       4
#define RTC_REGMEM_ADDR_1       5
#define RTC_REGMEM_ADDR_2       6
#define RTC_REGMEM_ADDR_3       7

#define RTC_ADDR0               0xc1108740 //RTC register  RTC_ADDR0
#define RTC_ADDR1               0xc1108744
#define RTC_ADDR2               0xc1108748
#define RTC_ADDR3               0xc110874C
#define RTC_ADDR4               0xc1108750

#define     RTC_REG0                (volatile unsigned long *)RTC_ADDR0
#define     RTC_REG1                (volatile unsigned long *)RTC_ADDR1
#define     RTC_REG2                (volatile unsigned long *)RTC_ADDR2
#define     RTC_REG3		            (volatile unsigned long *)RTC_ADDR3


#define     C_TIMER_MUX             (volatile unsigned long *)0xc1109940
#define     C_TIMERE                (volatile unsigned long *)0xc1109954


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
#define RTC_GPO_COUNTER_BITS    20
#define RTC_SER_ADDR_BITS       3
#define RTC_SER_DATA_BITS       32
#define RTC_USE_REGMEM          4*32
#define RTC_REGMEM_NUM          4

#define     set_rtc_TEST_BYPASS(a)  ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 3)) | (a << 3)))
#define     set_rtc_TEST_CLK(a)     ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 4)) | (a << 4)))
#define     set_rtc_TEST_MODE(a)    ((*RTC_REG0) = (((*RTC_REG0) & ~(1 << 5)) | (a << 5)))

#define sdi_ss                     (1<<2)
#define sen_ss                     (1<<18)
#define sclk_ss                    (1<<0)
#define sbus_ss                    (sen_ss|sclk_ss|sdi_ss)

#define RTC_sbus_LOW_ss(x)            ((*(volatile unsigned *)RTC_ADDR0) &= ~sbus_ss)
#define RTC_sdi_HIGH_ss(x)            ((*(volatile unsigned *)RTC_ADDR0) |=  sdi_ss )
#define RTC_sdi_LOW_ss(x)             ((*(volatile unsigned *)RTC_ADDR0) &= ~sdi_ss )
#define RTC_sen_HIGH_ss(x)            ((*(volatile unsigned *)RTC_ADDR0) |=  sen_ss )
#define RTC_sen_LOW_ss(x)             ((*(volatile unsigned *)RTC_ADDR0) &= ~sen_ss )
#define RTC_sclk_HIGH_ss(x)           ((*(volatile unsigned *)RTC_ADDR0) |=  sclk_ss)
#define RTC_sclk_LOW_ss(x)            ((*(volatile unsigned *)RTC_ADDR0) &= ~sclk_ss)

#define RTC_UNIQUE_CODE_BITS    32//64/2
#define RTC_STATIC_DATA_BITS    8

#endif