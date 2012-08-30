#include "pdown_boot.h"

#define PDOWN_FLAG 2009

#define GPIO_KEY
#define ADC_KEY
#define REMOTE_KEY

#ifdef REMOTE_KEY
#define REMOTE_KEY_VAL 0xeb
#endif

#ifdef ADC_KEY
unsigned char adc_key_num = 5;//quantity of adc key
float adc_res_ratio[] = { 0.15, 0.32, 0.45, 0.6, 0.75};//resistance ratio.MIN->MAX.recommend 0.1->0.9.
unsigned char adc_check_num = 3;//adc check times.recommend>=3.
//return key: 0(none),1(0v),2(3.3v),3(array[0]),4(array[1]),...
#endif

#ifdef REMOTE_KEY
static void init_ir(void)
{
    wr(0x1200100, 0x01d801ac);
    wr(0x1200104, 0x00f800ca);
    wr(0x1200108, 0x007a0066);
    wr(0x120010c, 0x0044002c);
    wr(0x1200110, 0x00fa0011 | (3<<28));
    wr(0x1200118, 0x08915d00);
    wr(0x120011c, 0x02213e70);
    
    wr(0x12000bc, rd(0x12000bc) | (1<<18)); //set GPIOB6 to remote
}
#endif

static void init_rtc(void)
{
    static_reg_write(0x4);
    wr(RTC_ADDR0, (rd(RTC_ADDR0) & ~(1<<16)));
}

void check_wake_up()
{
    unsigned adc_key_val;
    while(1)
    {
#ifdef ADC_KEY
        adc_key_val = adc_key_get();//return key: 0(none),1(0v),2(3.3v),3(array[0]),4(array[1]),...
#endif
            
        if((*(RTC_REG1) & (1<<3))//check rtc gpo state
#ifdef GPIO_KEY
        || (!(rd(0x01200068) & (1<<9)))//check gpio key
#endif
#ifdef ADC_KEY
        || (adc_key_val == 7)//check adc key
#endif
#ifdef REMOTE_KEY
        || ((rd(0x1200118) & (1<<3)) && (rd(0x1200114)>>24 == REMOTE_KEY_VAL))//check remote
#endif
          )
        {
            break;
        }
    }
}

void main_pdown(void)
{    
    if(irtc_read(RTC_REGMEM_ADDR_3) == PDOWN_FLAG)
    {
        rd(0x1100574) &= ~(1<<8);//the ARC clock and the MPEG system clock are connected to the 27Mhz crystal
        
        wr(0x1101c00, 0xD8);//sdram refresh clock(8us<10us)

/******disable useless clock gating******/        
        wr(0x1100540, 0x4580);
        wr(0x1100544, 0x3872);
        wr(0x1100548, 0x62af);
        wr(0x110054c, 0xf01f);
        wr(0x1100550, 0);
        wr(0x1100554, 1);
/****************************************/
        
#ifndef ADC_KEY
//disable adc clock gating
        wr(0x1100540, (rd(0x1100540) & ~(1<<10)));
        wr(0x1100548, (rd(0x1100548) & ~(1<<13)));
        wr(0x1100554, 0);
#endif
#ifndef REMOTE_KEY
//disable remote clock gating
        wr(0x1100540, (rd(0x1100540) & ~(1<<8)));
#endif

#ifdef REMOTE_KEY        
        init_ir();//init remote
#endif
        init_rtc();//init rtc
#ifdef ADC_KEY
        adc_init(TADC_CH11, 0);//init adc
#endif
        
        delay_us(20000);

        check_wake_up();//check condition to wake up
        
        irtc_write(RTC_REGMEM_ADDR_3, 0);//write power down flag into rtc reg
        irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);//resume rtc gpo to high level
        wr(0x1000448, 0x400001);//set watch dog reset system immediately
    }
}