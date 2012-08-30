#include "pdown_app.h"

#define PDOWN_FLAG 2009

//#define NIKED3_V3_8D
#ifdef NIKED3_V3_8D
#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
unsigned short my_task_id = 176;
#endif

void run_pdown_app(void)
{
    irtc_write(RTC_REGMEM_ADDR_3, PDOWN_FLAG);
    wr(0x1000448, 0x400001);
}

void set_wakeup_time(unsigned sec)
{
    irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
    irtc_write(RTC_GPO_COUNTER_ADDR, (0x600000 | (sec & 0xfffff)));//(0-19bit seconds)
}

#ifdef NIKED3_V3_8D
static void MyTask(void *arg)
{
    unsigned char flag = 0;
    
    while(1)
    {
        if(!(rd(0x01200068) & (1<<9)) && flag == 0)
        {
            AVTimeDly(20);
            if(!(rd(0x01200068) & (1<<9)))
            {
                flag = 1;
            }
        }
        else if((rd(0x01200068) & (1<<9)) && flag == 1)
        {
            AVTimeDly(20);
            if(rd(0x01200068) & (1<<9))
            {
                run_pdown_app();
            }
        }
        
        AVTimeDly(30);
    }    
}
#endif

void init_pdown_app(void)
{
    irtc_write(RTC_REGMEM_ADDR_3, 0);
#ifdef NIKED3_V3_8D
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],16, &my_task_id);
#endif
}