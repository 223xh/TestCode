#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))
/***COPIA 132M***/
#define DEFAULT_CLK 0x0958
/****************/
//19*3=57M
#define MAX_STEP 19

#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
static OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

static void my_set_freq(unsigned pll, unsigned div)
{
    unsigned tmp;
    
    tmp = rd(0x1100568);
    tmp &= ~(0x7fff);
    tmp |= (0x7fff & pll);
    wr(0x1100568, tmp);
    
    tmp = rd(0x1100574);
    tmp &= ~0xf;
    tmp |= (div & 0xf);
    wr(0x1100574, tmp);
}

static unsigned my_get_freq(void)
{
    unsigned m, n, div, od, clk;
    
    m = rd(0x1100568) & 0xff;
    n = ((rd(0x1100568)>>8) & 0x1f);
    od = ((rd(0x1100568)>>13) & 0x3);    
    div = rd(0x1100574) & 0xf;
    
    clk = 27 * m / n / (div + 1) / (unsigned)pow(2.0, (double)od);
    
    return clk;
}

static unsigned Get_Key_Value(void)
{
    return ((rd(0x12000a8)>>5) & 0x7);
}

static void MyTask(void *arg)
{
    unsigned n = 0;
    unsigned char flag = 0;
    char str[50];
    while(1)
    {
        if(Get_Key_Value() == 1)
        {
            AVTimeDly(20);
            if(Get_Key_Value() == 1)
            {
                flag = 1;
                n = n<MAX_STEP?n+1:MAX_STEP;
            }
        }
        else if(Get_Key_Value() == 4)
        {
            AVTimeDly(20);
            if(Get_Key_Value() == 4)
            {
                flag = 1;
                n = n>0?n-1:0;
            }
        }
        
        if(flag)
        {
            flag = 0;
            memset(str, 0, 50);
            
            my_set_freq(DEFAULT_CLK + 2*n, 1);
            
            sprintf(str, "CLOCK is [%u]MHz", my_get_freq());
            AWClearRectTxt(0,0,800,600);
            AWAsciiPrintf(200, 200, str);
        }
        
        AVTimeDly(200);
    }
}

void Init_Sdram_Test_COPIA_Display(void)
{
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],16, &my_task_id);
}