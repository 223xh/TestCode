#include <includes.h>
#include <chipsupport.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

static unsigned CGM0, CGM1, CGM2, CGM3, CGO0, CGO1;

static void UserDefineShutDown()
{   
    power_off_backlight();
    //rd(0x12000a4) |= 1<<2;    
}

static void UserDefineWakeUp()
{
    //rd(0x12000a4) &= ~(1<<2);
    power_on_backlight();
}

void PowerDown()
{
    OS_CPU_SR  cpu_sr;
    
    UserDefineShutDown();
    
    rd(0x1102df8) &= ~(1<<0);//Power down video dac
    
    CGM0 = rd(0x1100540);
    CGM1 = rd(0x1100544);
    CGM2 = rd(0x1100548);
    CGM3 = rd(0x110054c);
    CGO0 = rd(0x1100550);
    CGO1 = rd(0x1100554);
    
    wr(0x1100540, 0x180);
    wr(0x1100544, 0x3872);
    wr(0x1100548, 0x42af);
    wr(0x110054c, 0xf01f);
    wr(0x1100550, 0);
    wr(0x1100554, 0);
    
    rd(0x1100578) |= 1<<7;//sleep audio pll
    rd(0x110057c) |= 1<<15;//sleep video pll
    delay_10us(2000);
    
    rd(0x1200080) |= 1<<0;
    
    OS_ENTER_CRITICAL();
    wr(0x1101c00, 0xd8);
    rd(0x1100574) &= ~(1<<6);
    delay_10us(2000);
		OS_EXIT_CRITICAL();
}

void PowerUp()
{
    OS_ENTER_CRITICAL();
    rd(0x1100574) |= (1<<6);
    wr(0x1101c00, 0x500);
    delay_10us(2000);
		OS_EXIT_CRITICAL();
    
    rd(0x1200080) &= ~(1<<0);
    
    rd(0x1100578) &= ~(1<<7);
    rd(0x110057c) &= ~(1<<15);
    
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