#if 1
#include <includes.h>
#include <chipsupport.h>
#include "ioapi.h"

#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

#define MASK_R 0xc00
#define MASK_G 0xa00
#define MASK_B 0x900

static unsigned g_flag = 0;
static float coeff_r = 1.0;
static float coeff_g = 1.0;
static float coeff_b = 1.0;

static int g_sat = 0;
static int g_hue = 0;

static void program_gamma(int color, unsigned short *gamma_t, float coeff)
{
    int i;
		
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
                
    for (i=0;i<256;i++)
    {
       while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
       
       *(volatile unsigned *)0x01101204 = (unsigned short)(gamma_t[i]*coeff);        
    }
    
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}


static void Adjust_video_Para(void)
{
    INT32U video_adjustment;
    video_adjustment_t adjust_video_para;
    
    adjust_video_para.Brightness = 0;
    adjust_video_para.Contrast =  0;
    adjust_video_para.Saturation = g_sat;
    adjust_video_para.Hue = g_hue;
    video_adjustment = open("/dev/displayctl",O_RDWR);
    ioctl(video_adjustment, DISPCTLIO_ADJUST_VIDEO, &adjust_video_para);
    close(video_adjustment);
}

static void MyTask(void *arg)
{
    AVTimeDly(3000);
    while(1)
    {
        if(g_flag == 1)
        {
            g_flag = 0;
            program_gamma(MASK_R, GammaTableR, coeff_r);
            program_gamma(MASK_G, GammaTableG, coeff_g);
            program_gamma(MASK_B, GammaTableB, coeff_b);
        }
        else if(g_flag == 2)
        {
            g_flag = 0;
            Adjust_video_Para();
        }
        
        AVTimeDly(100);
    }
    //AVTaskDel(my_task_id);
}

void Init_Tune_Gamma()
{
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);    
}
#endif