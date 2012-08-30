#include <includes.h>
#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include <amsysdef.h>
#include <interrupt.h>

#define GPIO_EDGE_POL (*(volatile unsigned *)0x010003e0)
#define GPIO_0_3_SEL (*(volatile unsigned *)0x010003e4)
#define GPIO_4_7_SEL (*(volatile unsigned *)0x010003e8)
#define GPIO_8_11_SEL (*(volatile unsigned *)0x010003ec)
#define GPIO_12_15_SEL (*(volatile unsigned *)0x010003f0)
#define GPIO_FILTER_SEL_0_7 (*(volatile unsigned *)0x010003f4)
#define GPIO_FILTER_SEL_8_15 (*(volatile unsigned *)0x010003f8)

static int g_count = 0;
static void gpio_irq_handler(void *arg)
{
    #if 0
    ++g_count;
    if(g_count >= 60)
    {
        printf("g=%u[%u]\n", g_count, AVTimeGet());
        g_count = 0;
    }
    #else
    printf("IOMUX falling edge[%u]\n", AVTimeGet());
    #endif
}

static int t_handle = -1;
void Init_Test_IRQ()
{
    *(volatile unsigned*)0x12000b8 &= ~0xf;
    
    GPIO_EDGE_POL = (1<<16) | 1;
    
    //GPIO_0_3_SEL = 44;//STV1;
    GPIO_0_3_SEL = 51;//IOMUX3;
    
    if(t_handle < 0)
        t_handle = AVRequestIrq(ISR_TYPE_GPIO , 1, gpio_irq_handler, NULL);
    
    if(t_handle < 0)    
        printf("\nRequest GPIO IRQ failed.\n");
    else
        printf("\nRequest GPIO IRQ OK.\n");
}