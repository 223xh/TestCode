#include <includes.h>
#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include <amsysdef.h>
#include <interrupt.h>

#define IRQ_VERTSYNC (1<<3)

#define TASK_STK_SIZE 1024
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
unsigned short my_task_id = 66;

int VsyncHandle = -1;
//unsigned VsyncCounter = 0;

static void vsync_irq_handler(void *arg)
{
    //++VsyncCounter;
    if((*(volatile unsigned *)0x1200044) & (1<<30))
        *(volatile unsigned *)0x1200044 &= ~(1<<30);
    else
        *(volatile unsigned *)0x1200044 |= (1<<30);
}
/*
static void MyTask(void *arg)
{
    OS_CPU_SR  cpu_sr;
    
    AVTimeDly(5000);
    OS_ENTER_CRITICAL();
    printf("start get Vsync counter\n");
    OS_EXIT_CRITICAL();
    AVTimeDly(100);
    while(1)
    {
        OS_ENTER_CRITICAL();
        printf("Vsync counter=[%u]\n", VsyncCounter);
        OS_EXIT_CRITICAL();
        AVTimeDly(1000);
    }
}
*/
static void Init_TestIO()
{
    //IOMUX3 P3
    *(volatile unsigned *)0x12000b8 &= ~(0xf<<0);
    
    *(volatile unsigned *)0x1200040 &= ~(1<<30);//OE
}

void Init_Vsync_IRQ()
{
#ifndef __ROM_
    OS_CPU_SR  cpu_sr;
#endif

    Init_TestIO();
    
    if(VsyncHandle < 0)
        VsyncHandle = AVRequestIrq(ISR_TYPE_GENERAL, IRQ_VERTSYNC, vsync_irq_handler, NULL);

#ifndef __ROM_    
    OS_ENTER_CRITICAL();
    if(VsyncHandle < 0)    
        printf("\nRequest Vsync IRQ failed.\n");
    else
        printf("\nRequest Vsync IRQ OK.\n");
    OS_EXIT_CRITICAL();
#endif       
    //AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
}