#include <includes.h>
#include <chipsupport.h>

#define KEEP_ADC_KEY
#define KEEP_REMOTE_KEY

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

static unsigned CGM0, CGM1, CGM2, CGM3, CGO0, CGO1;

static void UserDefineShutDown()
{
    power_off_backlight();
/***add customer function***/

/***************************/
}

static void UserDefineWakeUp()
{
/***add customer function***/

/***************************/
    power_on_backlight();
}

void PowerDown()
{
    OS_CPU_SR  cpu_sr;
    
    UserDefineShutDown();
    
    rd(0x1102df8) &= ~(1<<0);//Power down video dac
    
    rd(0x1200294) &= ~(1<<0);//Disable VBG_EN(If adc module has any problem, remark it.)
    	    
    rd(0x1101588) &= ~(1<<2);//Power down audio dac(50+mA)
    
    CGM0 = rd(0x1100540);
    CGM1 = rd(0x1100544);
    CGM2 = rd(0x1100548);
    CGM3 = rd(0x110054c);
    CGO0 = rd(0x1100550);
    CGO1 = rd(0x1100554);
    
/******disable useless clock gating******/        
        wr(0x1100540, 0x1580);
        wr(0x1100544, 0x3870);
        wr(0x1100548, 0x62af);
        wr(0x110054c, 0xf01b);
        wr(0x1100550, 0);
        wr(0x1100554, 0);
/****************************************/
        
#ifndef KEEP_ADC_KEY
//disable adc clock gating
        rd(0x1100540) &= ~(1<<12);//MPEG_DOMAIN Peripherals 12
        rd(0x1100548) &= ~(1<<13);//MPEG_DOMAIN AIU 45
#endif
#ifndef KEEP_REMOTE_KEY
//disable remote clock gating
        rd(0x1100540) &= ~(1<<8);
#endif

    rd(0x110056c) |= 1<<15;//sleep audio pll
    rd(0x1100570) |= 1<<15;//sleep video pll
    delay_10us(2000);
        
    rd(0x1200080) |= 1<<0;//power down usb
    
    OS_ENTER_CRITICAL();
    wr(0x1101c00, 0xd8);//sdram refresh clock(8us<10us)
    rd(0x1100574) &= ~(1<<8);//set system clock connect to 27Mhz crystal
    delay_10us(2000);
		OS_EXIT_CRITICAL();
}

void PowerUp()
{
    OS_ENTER_CRITICAL();
    rd(0x1100574) |= (1<<8);
    wr(0x1101c00, 0x500);
    delay_10us(2000);
		OS_EXIT_CRITICAL();
    
    rd(0x1200080) &= ~(1<<0);
    
    rd(0x1100570) &= ~(1<<15);
    rd(0x110056c) &= ~(1<<15);
        
    wr(0x1100540, CGM0);
    wr(0x1100544, CGM1);
    wr(0x1100548, CGM2);
    wr(0x110054c, CGM3);
    wr(0x1100550, CGO0);
    wr(0x1100554, CGO1);
    
    UserDefineWakeUp();
}

void PowerUpReset()
{
    wr(0x1000448, 0x400001);
}