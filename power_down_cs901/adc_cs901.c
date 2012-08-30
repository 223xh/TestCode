#include "adc_cs901.h"

#define TOTAL_SAMPLES	16
#define CUT_OFF_SAMPLES	2

static int ref_high,ref_low, ref_valid_low, ref_valid_high;

extern void delay_us(unsigned long us);

static unsigned short adc_manual_sample(unsigned char ch)
{
	unsigned char i = 0, j = 0;
	unsigned short data[TOTAL_SAMPLES], temp = 0;
	
	//Preset chan list
	(*ADC_TSCREEN_CHAN_LIST) = 0<<24;
	(*ADC_TSCREEN_CHAN_LIST) |= ch<<0;
	//Delay 3mS for LPF of PWM to reach the stable DC level
	//AVTimeDly(6);
	delay_us(30);
	//Sample
	for (i=0; i<TOTAL_SAMPLES; i++)
	{
		ADC_start_sample;
		//Wait for fully stopped
		while ((*ADC_TSCREEN_REG0)&0x70000000);
		//Empty FIFO
		while (((*ADC_TSCREEN_REG0)>>21)&0x1f)
			data[i] = (unsigned short)((*ADC_TSCREEN_FIFO_RD)&0x0000ffff) & 0x3ff;
		//Delay 1mS
		//AVTimeDly(1);
		delay_us(10);
	}
	//Re-order HIGH & LOW sample data
	for (i=0; i<TOTAL_SAMPLES-1; i++)
	{
		for (j=1; j<TOTAL_SAMPLES; j++)
		{
			if (data[i] > data[j])
			{
				temp = data[i];
				data[i] = data[j];
				data[j] = temp;
			}
		}
	}
	//Take average midian
	temp = 0;
	for (i=CUT_OFF_SAMPLES; i<TOTAL_SAMPLES-CUT_OFF_SAMPLES; i++)
		temp += data[i];
	temp += (TOTAL_SAMPLES - CUT_OFF_SAMPLES - CUT_OFF_SAMPLES) >> 1;
	temp /= (TOTAL_SAMPLES - CUT_OFF_SAMPLES - CUT_OFF_SAMPLES);
	return temp;
}

static void adc_cal()
{    
    unsigned chan_list = 0, Aux10 = 0, Aux72 = 0;
    unsigned char oev_en_lcd = 0, oev_en_gpio = 0, oev_level_gpio =0;
    
    //Backup chan list & ADC Aux
    chan_list = (*ADC_TSCREEN_CHAN_LIST);
    Aux10 = (*ADC_TSCREEN_CHAN_10_SW);
    Aux72 = (*ADC_TSCREEN_AUX_SW);
    //Pick out do-able chan for calibration	
    ref_high = adc_manual_sample(5);
    ref_low = adc_manual_sample(4);
    
    //Backup OEV pin status
    oev_en_lcd = ((*(volatile unsigned *)0x012000B0)>>11) & 1;
    oev_en_gpio = ((*(volatile unsigned *)0x01200030)>>6) & 1;
    oev_level_gpio = ((*(volatile unsigned *)0x01200034)>>6) & 1;
    //Enable OEV to GPIO OUTPUT mode
    *(volatile unsigned *) 0x012000B0 &= ~(1<<11);
    *(volatile unsigned *) 0x01200030 &= ~(1<<6);
    //OEV outputs LOW to measure 1/7 of 3.3V ADC readings
    *(volatile unsigned *) 0x01200034 &= ~(1<<6);
    ref_valid_low = adc_manual_sample(6);   // get readings of 1/7*3.3V
    //OEV outputs HIGH to measure 6/7 of 3.3V readings
    *(volatile unsigned *) 0x01200034 |= (1<<6);
    ref_valid_high = adc_manual_sample(6);   // get readings of 6/7*3.3V
    //Restore OEV pin status
    if (oev_en_lcd)
        *(volatile unsigned *) 0x012000B0 |=  (1 << 11);
    else
        *(volatile unsigned *) 0x012000B0 &= ~(1 << 11);
    if (oev_en_gpio)
        *(volatile unsigned *) 0x01200030 |=  (1 <<  6);
    else
        *(volatile unsigned *) 0x01200030 &= ~(1 <<  6);
    if (oev_level_gpio)
        *(volatile unsigned *) 0x01200034 |=  (1 <<  6);
    else
        *(volatile unsigned *) 0x01200034 &= ~(1 <<  6);
    
    //Restore init settings for chan list & ADC Aux
    (*ADC_TSCREEN_CHAN_LIST) = chan_list;
    (*ADC_TSCREEN_CHAN_10_SW) = Aux10;
    (*ADC_TSCREEN_AUX_SW) = Aux72;
}

void adc_init(unsigned ch)
{    
    ADC_enable_band_gap;
    (*ADC_TSCREEN_REG0          ) &= 0xffff0000;
    (*ADC_TSCREEN_CHAN_LIST     )  = ch;//0x00000004; //ch4
    (*ADC_TSCREEN_AVG_CNTL      )  = 0;//0x02000300; //ch4 median averaging of 8 samples
    (*ADC_TSCREEN_REG3          )  = ( 1 << 21) | (20 << 10); //enable ADC analog block & Set ADC clock = 1.2857MH & Block delay = 0uS.
    (*ADC_TSCREEN_DELAY         )  = ( 3 << 24) |       // Input: 100uS timebase
                                     ( 9 << 16) |       // Input: 9+1 counts = 1mS
                                     ( 3 <<  8) |       // Sample: 1mS timebase
                                     ( 9 <<  0) ;       // Sample: 9+1 counts = 10mS
    (*ADC_TSCREEN_AUX_SW        )  = 0x008fac1c;//((ch & 0x7) << 14);// | 0x0000001c;
    (*ADC_TSCREEN_CHAN_10_SW    )  = 0x001c001c;
    (*ADC_TSCREEN_DETECT_IDLE_SW)  = 0x001c001c;
    (*ADC_TSCREEN_DELTA_10      )  = 0x00000000;
    ADC_enable_sample;
    adc_cal();
}

static unsigned get_adc_value(void)
{
	unsigned data=0;	

	// Stop sampling
	(*ADC_TSCREEN_REG0)   |= (1 << 14);
	// Wait for ADC sampling fully stopped
	while ((*ADC_TSCREEN_REG0)&0x70000000);
	// Empty FIFO
	while (((*ADC_TSCREEN_REG0)>>21)&0x1f)
		data = (*ADC_TSCREEN_FIFO_RD)&0x3ff;
	// Start sampling
	(*ADC_TSCREEN_REG0)   |= (1 << 2);
	// Wait for new sample finished
	while (!(((*ADC_TSCREEN_REG0)>>21)&0x1f));
	// Read the 1st sample data
	data = (*ADC_TSCREEN_FIFO_RD)&0x3ff;
	// Stop sampling
	(*ADC_TSCREEN_REG0)   |= (1 << 14);
	// Wait for ADC sampling fully stopped
	while ((*ADC_TSCREEN_REG0)&0x70000000);
	
	return data;
}

extern unsigned char adc_key_num;
extern float adc_res_ratio[];
extern unsigned char adc_check_num;

static unsigned char check_num = 1;
static unsigned char last_key = 0xff;

unsigned adc_key_get(void)
{
    unsigned key_val = 0;
    unsigned adc_val;
    unsigned char i;
    
    adc_val = get_adc_value();
    
    if(adc_val < ref_valid_low)//0V
    {
        if(last_key == 1)
            ++check_num;
        else
        {
            last_key = 1;
            check_num = 1;
        }
        
        if(check_num >= adc_check_num)
            key_val = 1;
    }
    else if(adc_val > ref_valid_high)//3.3V
    {
        if(last_key == 2)
            ++check_num;
        else
        {
            last_key = 2;
            check_num = 1;
        }
        
        if(check_num >= adc_check_num)
            key_val = 2;
    }
    else
    {
        for(i=0;i<adc_key_num;i++)
        {
            if(i == 0)
            {
                if((adc_val > ref_valid_low) && 
                (adc_val < (unsigned)((adc_res_ratio[i] + adc_res_ratio[i + 1])*
                (ref_valid_high - ref_valid_low)/2.0*7.0/5.0)))
                    break;
            }
            else if(i == adc_key_num - 1)
            {
                if((adc_val < ref_valid_high) && 
                (adc_val > (unsigned)((adc_res_ratio[i] + adc_res_ratio[i - 1])*
                (ref_valid_high - ref_valid_low)/2.0*7.0/5.0)))
                    break;
            }
            else
            {
                if((adc_val > (unsigned)((adc_res_ratio[i] + adc_res_ratio[i - 1])*
                (ref_valid_high - ref_valid_low)/2.0*7.0/5.0)) && 
                (adc_val < (unsigned)((adc_res_ratio[i] + adc_res_ratio[i + 1])*
                (ref_valid_high - ref_valid_low)/2.0*7.0/5.0)))
                    break;
            }
        }
        
        if(last_key == i + 3)
            ++check_num;
        else
        {
            last_key = i + 3;
            check_num = 1;
        }
        
        if(check_num >= adc_check_num)
            key_val = i + 3;
    }
    
    return key_val;    
}