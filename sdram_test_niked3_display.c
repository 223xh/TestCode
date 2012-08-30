#include <includes.h>
#include <math.h>
#include "adc.h"
#include <chipsupport.h>
#include <af_engine.h>		
#include <af_message.h>		
#include <af_msgtype.h>		
#include <af_global.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))
/***Niked3 156M***/
#define DEFAULT_CLK 0x0c4e
/*****************/
//18*2=36M
#define MAX_STEP 18

#define ADC_MAX 820
#define ADC_MIN 20
#define ADC_COEFF_PLUS 0.75
#define ADC_COEFF_MINUS 0.6
#define ADC_VAL_PLUS (unsigned short)((ADC_MAX-ADC_MIN)*ADC_COEFF_PLUS+ADC_MIN)
#define ADC_VAL_MINUS (unsigned short)((ADC_MAX-ADC_MIN)*ADC_COEFF_MINUS+ADC_MIN)
#define ADC_VALID_WIDTH 20
#define ADC_HIGH_PLUS ((ADC_VAL_PLUS+ADC_VALID_WIDTH)>ADC_MAX?ADC_MAX:(ADC_VAL_PLUS+ADC_VALID_WIDTH))
#define ADC_LOW_PLUS ((ADC_VAL_PLUS-ADC_VALID_WIDTH)<ADC_MIN?ADC_MIN:(ADC_VAL_PLUS-ADC_VALID_WIDTH))
#define ADC_HIGH_MINUS ((ADC_VAL_MINUS+ADC_VALID_WIDTH)>ADC_MAX?ADC_MAX:(ADC_VAL_MINUS+ADC_VALID_WIDTH))
#define ADC_LOW_MINUS ((ADC_VAL_MINUS-ADC_VALID_WIDTH)<ADC_MIN?ADC_MIN:(ADC_VAL_MINUS-ADC_VALID_WIDTH))

typedef unsigned char bool;

extern void adc_init(unsigned ch);
extern bool adc_check(unsigned threshold_high, unsigned threshold_low);

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
    tmp &= ~0x7f;
    tmp |= (div & 0x7f);
    wr(0x1100574, tmp);
}

static unsigned my_get_freq(void)
{
    unsigned m, n, div, od, clk;
    
    m = rd(0x1100568) & 0x1ff;
    n = ((rd(0x1100568)>>9) & 0x1f);
    od = ((rd(0x1100568)>>14) & 0x1);    
    div = rd(0x1100574) & 0x7f;
    
    clk = 24 * m / n / (div + 1) / (unsigned)pow(2.0, (double)od);
    
    return clk;
}

static void MyTask(void *arg)
{
    unsigned n = 0;
    unsigned char flag = 0;
    char str[50];
#ifndef __ROM_
    printf("Sdram test task is running.\n");
#endif
    while(1)
    {
        adc_init(TADC_CH11);
        if(adc_check(ADC_HIGH_PLUS, ADC_LOW_PLUS))
        {
            AVTimeDly(10);
            if(adc_check(ADC_HIGH_PLUS, ADC_LOW_PLUS))
            {
                flag = 1;
                n = n<MAX_STEP?n+1:MAX_STEP;
#ifndef __ROM_
                printf("Press plus key.\n");
#endif
            }
        }
        else if(adc_check(ADC_HIGH_MINUS, ADC_LOW_MINUS))
        {
            AVTimeDly(10);
            if(adc_check(ADC_HIGH_MINUS, ADC_LOW_MINUS))
            {
                flag = 1;
                n = n>0?n-1:0;
#ifndef __ROM_
                printf("Press minus key.\n");
#endif
            }
        }
        
        if(flag)
        {
            flag = 0;
            memset(str, 0, 50);
            
            my_set_freq(DEFAULT_CLK + n, 1);
#ifndef __ROM_
            printf("CLOCK is [%u]MHz\n",my_get_freq());
#endif
            sprintf(str, "CLOCK is [%u]MHz", my_get_freq());
            af_osd_draw_rect(OSD_LAYER_HW_LAYER0,200,200,360,50, 0, 1);
            af_osd_draw_string(OSD_LAYER_HW_LAYER0, str,strlen(str),210,210,2, af_get_default_font_group_id());
        }
        
        AVTimeDly(200);
    }
}

void Init_Sdram_Test_NIKED3_Display(void)
{
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],16, &my_task_id);
}