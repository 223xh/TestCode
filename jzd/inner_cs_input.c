#define rd(reg)	      *(volatile unsigned*)(0xc1100000+(reg<<2))
#define wr(reg, val)  rd(reg) = val
#define REG0    0x202c    //0xC11080B0
#define REG1    (REG0+1)
#define REG2    (REG1+1)
#define REG3    (REG2+1)
#define REG4    (REG3+1)
#define REG5    (REG4+1)
#define REG6    (REG5+1)
#define REG7    (REG6+1)
#define REG8    (REG7+1)
#define REG9    (REG8+1)
#define REG10   (REG9+1)
#define REG11   (REG10+1)
#define REG12   (REG11+1)

#define PWM_TCNT        (600-1)
#define PWM_MAX_VAL    (420)

#define P_PERIPHS_PIN_MUX_0     (volatile unsigned long *)0xc11080b0
#define P_PREG_AM_ANALOG_ADDR   (volatile unsigned long *)0xc110810c

#define P_LED_PWM_REG0          (volatile unsigned long *)0xc1108768
#define P_LED_PWM_REG1          (volatile unsigned long *)0xc110876c
#define P_LED_PWM_REG2          (volatile unsigned long *)0xc1108770
#define P_LED_PWM_REG3          (volatile unsigned long *)0xc1108774 //0x21dd
#define P_LED_PWM_REG4          (volatile unsigned long *)0xc1108778
#define P_VGHL_PWM_REG0          (volatile unsigned long *)0xc1108780 //0x21e0
#define P_VGHL_PWM_REG1          (volatile unsigned long *)0xc1108784
#define P_VGHL_PWM_REG2          (volatile unsigned long *)0xc1108788
#define P_VGHL_PWM_REG3          (volatile unsigned long *)0xc110878c //0x21dd
#define P_VGHL_PWM_REG4          (volatile unsigned long *)0xc1108790

/*
 * 函数名：inner_cs_input_level
 * 用途：使用内置VGHL B/L的反馈CS0/1做输入.
 * 平台: AML8726-M_DEV_BOARD 2010-10-11_V1.0
 * 引脚：P4(VGHL_CS0/1)   T2(LED_CS1)  T1(LED_CS0)
 * 注意：输入脚的高电平需大于0.7V，低电平需小于0.4V
 * 入口参数：无
 * 返回参数：
       level[12]  <-->  VGHL_CS1上的逻辑电平
       level[ 8]  <-->  VGHL_CS0上的逻辑电平
       level[ 4]  <-->  LED_CS1上的逻辑电平
       level[ 0]  <-->  LED_CS0上的逻辑电平
 */
unsigned int inner_cs_input_level(void)
{
    unsigned int level = 0;
    unsigned int cs_no = 0;
    //pin64 LED_CS0
    rd(REG0) &= ~(3<<21);
    // Enable VBG_EN
    (*P_PREG_AM_ANALOG_ADDR) |= (1 << 0);
    // pin mux
    // wire            pm_gpioA_7_led_pwm          = pin_mux_reg0[22];
    (*P_LED_PWM_REG0) = 0;
    (*P_LED_PWM_REG1) = 0;
    (*P_LED_PWM_REG2) = 0;
    (*P_LED_PWM_REG3) = 0;
    (*P_LED_PWM_REG4) = 0;
    (*P_LED_PWM_REG0)  =    (0 << 31)           |       // disable the overall circuit
                            (0 << 30)           |       // 1:Closed Loop  0:Open Loop
                            (PWM_TCNT << 16)    |       // PWM total count
                            (0 << 13)           |       // Enable
                            (1 << 12)           |       // enable
                            (0 << 10)           |       // test
                            (7 << 7)            |       // CS0 REF, Voltage FeedBack: about 0.505V
                            (7 << 4)            |       // CS1 REF, Current FeedBack: about 0.505V
                            (0 << 0);                   // DIMCTL Analog dimmer
     (*P_LED_PWM_REG1)  =    (1 << 30)           |       // enable high frequency clock
                            (PWM_MAX_VAL << 16) |       // MAX PWM value
                            (0  << 0);                  // MIN PWM value
     (*P_LED_PWM_REG2)  =    (0 << 31)       |       // disable timeout test mode
                            (0 << 30)       |       // timeout based on the comparator output
                            (0 << 16)       |       // timeout = 10uS
                            (0 << 13)       |       // Select oscillator as the clock (just for grins)
                            (1 << 11)       |       // 1:Enable OverCurrent Portection  0:Disable
                            (3 << 8)        |       // Filter: shift every 3 ticks
                            (0 << 6)        |       // Filter: count 1uS ticks
                            (0 << 5)        |       // PWM polarity : negative
                            (0 << 4)        |       // comparator: negative, Different with NikeD3
                            (1 << 0);               // +/- 1
    (*P_LED_PWM_REG3)  =  (   1 << 16) |    // Feedback down-sampling = PWM_freq/1 = PWM_freq
                          (   1 << 14) |    // enable to re-write MATCH_VAL
                          (   210 <<  0) ;  // preset PWM_duty = 50%
    (*P_LED_PWM_REG4)  =  (   0 << 30) |    // 1:Digital Dimmer  0:Analog Dimmer
                          (   2 << 28) |    // dimmer_timebase = 1uS
                          (1000 << 14) |    // Digital dimmer_duty = 0%, the most darkness
                          (1000 <<  0) ;    // dimmer_freq = 1KHz
    cs_no = *(P_LED_PWM_REG3);
    if(cs_no &(1<<14))
      level |= (1<<0);
    if(cs_no &(1<<15))
      level |= (1<<4);
      
    (*P_VGHL_PWM_REG0) = 0;
    (*P_VGHL_PWM_REG1) = 0;
    (*P_VGHL_PWM_REG2) = 0;
    (*P_VGHL_PWM_REG3) = 0;
    (*P_VGHL_PWM_REG4) = 0;
    (*P_VGHL_PWM_REG0)  =    (0 << 31)           |       // disable the overall circuit
                            (0 << 30)           |       // 1:Closed Loop  0:Open Loop
                            (PWM_TCNT << 16)    |       // PWM total count
                            (0 << 13)           |       // Enable
                            (1 << 12)           |       // enable
                            (0 << 10)           |       // test
                            (7 << 7)            |       // CS0 REF, Voltage FeedBack: about 0.505V
                            (7 << 4)            |       // CS1 REF, Current FeedBack: about 0.505V
                            (0 << 0);                   // DIMCTL Analog dimmer
     (*P_VGHL_PWM_REG1)  =    (1 << 30)           |       // enable high frequency clock
                            (PWM_MAX_VAL << 16) |       // MAX PWM value
                            (0  << 0);                  // MIN PWM value
     (*P_VGHL_PWM_REG2)  =    (0 << 31)       |       // disable timeout test mode
                            (0 << 30)       |       // timeout based on the comparator output
                            (0 << 16)       |       // timeout = 10uS
                            (0 << 13)       |       // Select oscillator as the clock (just for grins)
                            (1 << 11)       |       // 1:Enable OverCurrent Portection  0:Disable
                            (3 << 8)        |       // Filter: shift every 3 ticks
                            (0 << 6)        |       // Filter: count 1uS ticks
                            (0 << 5)        |       // PWM polarity : negative
                            (0 << 4)        |       // comparator: negative, Different with NikeD3
                            (1 << 0);               // +/- 1
    (*P_VGHL_PWM_REG3)  =  (   1 << 16) |    // Feedback down-sampling = PWM_freq/1 = PWM_freq
                          (   1 << 14) |    // enable to re-write MATCH_VAL
                          (   210 <<  0) ;  // preset PWM_duty = 50%
    (*P_VGHL_PWM_REG4)  =  (   0 << 30) |    // 1:Digital Dimmer  0:Analog Dimmer
                          (   2 << 28) |    // dimmer_timebase = 1uS
                          (1000 << 14) |    // Digital dimmer_duty = 0%, the most darkness
                          (1000 <<  0) ;    // dimmer_freq = 1KHz
    cs_no = *(P_VGHL_PWM_REG3);
    if(cs_no &(1<<14))
      level |= (1<<8);
    if(cs_no &(1<<15))
      level |= (1<<12);

    return level;
}