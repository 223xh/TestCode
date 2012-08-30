#include <includes.h>
#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include <amsysdef.h>
#include <interrupt.h>

#define IRQ_VERTSYNC (1<<3)

int VsyncHandle = -1;

static void vsync_irq_handler(void *arg)
{
    
}

void Init_Vsync_IRQ()
{
#ifndef __ROM_
    OS_CPU_SR  cpu_sr;
#endif
    
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
}