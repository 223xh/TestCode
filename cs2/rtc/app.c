#define RTC_REG2_CLOCK  	*(volatile unsigned *)(0x1200288)
#define RTC_REG3_CLOCK  	*(volatile unsigned *)(0x120028c)


unsigned long measure_32K_CLK81()
{
      RTC_REG2_CLOCK =0;
      RTC_REG3_CLOCK = (4095<<0) | (1<<18) ; //set 4096 clock
      RTC_REG3_CLOCK = RTC_REG3_CLOCK | (1<<16);  //start count
      while(RTC_REG3_CLOCK & (1<<20)){};
      RTC_REG3_CLOCK = RTC_REG3_CLOCK & ~((1<<16) | (1<<18));
      return RTC_REG2_CLOCK;
}