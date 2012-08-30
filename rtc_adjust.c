#include <includes.h>
#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include "irtc.h"

#define TASK_STK_SIZE 1024
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
unsigned short my_task_id = 66;

//#define HI_CHK_VAL 2000000000
#define HI_RANGE 90000
#define HI_UNIT ((double)4294967.295)

static unsigned rtc_base, hi_val = 0;
static unsigned av_start;
static unsigned char av_flag = 0;
static unsigned char count = 61;

void ResetRTCAdjust()
{
    OS_CPU_SR  cpu_sr;
    OS_ENTER_CRITICAL();
    rtc_base = irtc_read(RTC_COUNTER_ADDR);
    OS_EXIT_CRITICAL();
    //AVTimeSet(0);
    av_start = AVTimeGet();
    hi_val = 0;
    count = 61
    if(av_start < HI_RANGE)
        av_flag = 1;
    else
        av_flag = 0;
}

static void MyTask(void *arg)
{
    unsigned rtc_val, av_val;
    
    OS_CPU_SR  cpu_sr;
    
    static_reg_write(0x4);
    OS_ENTER_CRITICAL();
    irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
    irtc_write(RTC_SEC_ADJUST_ADDR, 0);    
    
    rtc_base = irtc_read(RTC_COUNTER_ADDR);
    OS_EXIT_CRITICAL();
    //AVTimeSet(0);
    av_start = AVTimeGet();
    if(av_start < HI_RANGE)
        av_flag = 1;
    else
        av_flag = 0;
    
    while(1)
    {
        if(--count)
            AVTimeDly(1000);
        else
        {
            count = 61;
            av_val = AVTimeGet();
            if(av_val < HI_RANGE)
            {
                if(!av_flag)
                {
                    av_flag = 1;
                    ++hi_val;
                }
            }
            else
            {
                if(av_flag)
                    av_flag = 0;
            }
            rtc_val = (unsigned)(rtc_base + HI_UNIT*hi_val + ((double)((int)(av_val - av_start))/1000));
            OS_ENTER_CRITICAL();
            irtc_write(RTC_COUNTER_ADDR, rtc_val);
            OS_EXIT_CRITICAL();
            
/*            
            if(av_val > HI_CHK_VAL)
            {
                if(!av_flag)
                    av_flag = 1;
            }
            else
            {
                if(av_flag)
                {
                    av_flag = 0;
                    ++hi_val;
                }
            }
*/
        }
        //AVTimeDly(1000);
    }
    //AVTaskDel(my_task_id);
}

void Init_RTC_Adjust()
{
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
}