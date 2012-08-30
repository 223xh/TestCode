#include "adc.h"

static int ref_high,ref_low, ref_0150mV, ref_3150mV;

static void adc_cal(unsigned cal_mask)
{
	unsigned char i = 0, j = 0, chan_high = 0, chan_low = 0;
	unsigned short data, data_high[8] = {0,0,0,0,0,0,0,0}, data_low[8] = {0,0,0,0,0,0,0,0};
	unsigned chan_list = 0, Aux10 = 0, Aux72 = 0;
	
	//Backup chan list & ADC Aux
	chan_list = (*ADC_TSCREEN_CHAN_LIST);
	Aux10 = (*ADC_TSCREEN_CHAN_10_SW);
	Aux72 = (*ADC_TSCREEN_AUX_SW);
	//Pick out do-able chan for calibration
	switch (cal_mask)
	{
		case 0:
		case 1:
		case 4:
		case 5:
			chan_high = 1;
			chan_low = 3;
			(*ADC_TSCREEN_AUX_SW) = 0x008fac19;
			(*ADC_TSCREEN_CHAN_10_SW) = 0x01990119;
			break;
		case 8:
		case 9:
			chan_high = 1;
			chan_low = 2;
			(*ADC_TSCREEN_AUX_SW) = 0x008fac1a;
			(*ADC_TSCREEN_CHAN_10_SW) = 0x019a011a;
			break;
		case 2:
		case 10:
			chan_high = 0;
			chan_low = 2;
			(*ADC_TSCREEN_AUX_SW) = 0x008fac16;
			(*ADC_TSCREEN_CHAN_10_SW) = 0x01960116;
			break;
		case 3:
		case 6:
		case 7:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		default:
			break;
	}
	//If do-able, re-cal cfg_ref_low & cfg_ref_high instead of preset value in UI code
	if (chan_high || chan_low)
	{
		//Preset chan list
		(*ADC_TSCREEN_CHAN_LIST) = 1<<24;
		(*ADC_TSCREEN_CHAN_LIST) |= chan_high<<0;
		(*ADC_TSCREEN_CHAN_LIST) |= chan_low<<3;
		//Sample 8 times for HIGH & LOW
		for (i=0; i<8; i++)
		{
			ADC_start_sample;
			//Wait for fully stopped
			while ((*ADC_TSCREEN_REG0)&0x70000000);
			//Empty FIFO
			while (((*ADC_TSCREEN_REG0)>>21)&0x1f)
			{
				data = (unsigned short)((*ADC_TSCREEN_FIFO_RD)&0x0000ffff);
				if (((data&0x7000)>>12) == chan_high)
					data_high[i] = data&0x3ff;
				if (((data&0x7000)>>12) == chan_low)
					data_low[i] = data&0x3ff;
			}
		}
		//Re-order HIGH & LOW sample data
		for (i=0; i<7; i++)
		{
			for (j=1; j<8; j++)
			{
				if (data_high[i] > data_high[j])
				{
					data = data_high[i];
					data_high[i] = data_high[j];
					data_high[j] = data;
				}
				if (data_low[i] > data_low[j])
				{
					data = data_low[i];
					data_low[i] = data_low[j];
					data_low[j] = data;
				}
			}
		}
		//Take average midian as the referenced HIGH & LOW value accordingly
		ref_high = 0;
		ref_low = 0;
		for (i=2; i<6; i++)
		{
			ref_high += data_high[i];
			ref_low += data_low[i];
		}
		ref_high += 2;
		ref_high >>= 2;
		ref_low += 2;
		ref_low >>= 2;
		//Calculate rough referenced 0.15V & 3.15V value accordingly, 5% is the portion for non-linearity of ADC
		ref_0150mV = ref_low  + (ref_high-ref_low)*5/100;
		ref_3150mV = ref_high - (ref_high-ref_low)*5/100;
	}
	//Restore init settings for chan list & ADC Aux
	(*ADC_TSCREEN_CHAN_LIST) = chan_list;
	(*ADC_TSCREEN_CHAN_10_SW) = Aux10;
	(*ADC_TSCREEN_AUX_SW) = Aux72;
}

void adc_init(unsigned ch, unsigned cal_mask)
{    
    ADC_enable_band_gap;
    (*ADC_TSCREEN_REG0          ) &= 0xffff0000;
    (*ADC_TSCREEN_CHAN_LIST     )  = 0x00000004; //ch4
    (*ADC_TSCREEN_AVG_CNTL      )  = 0x02000300; //ch4 median averaging of 8 samples
    (*ADC_TSCREEN_REG3          )  = ( 1 << 21) | (20 << 10); //enable ADC analog block & Set ADC clock = 1.2857MH & Block delay = 0uS.
    (*ADC_TSCREEN_DELAY         )  = ( 3 << 24) |       // Input: 100uS timebase
                                     ( 9 << 16) |       // Input: 9+1 counts = 1mS
                                     ( 3 <<  8) |       // Sample: 1mS timebase
                                     ( 9 <<  0) ;       // Sample: 9+1 counts = 10mS
    (*ADC_TSCREEN_AUX_SW        )  = ((ch & 0x7) << 14) | 0x0000001c;
    (*ADC_TSCREEN_CHAN_10_SW    )  = 0x001c001c;
    (*ADC_TSCREEN_DETECT_IDLE_SW)  = 0x001c001c;
    (*ADC_TSCREEN_DELTA_10      )  = 0x00000000;
    ADC_enable_sample;
    adc_cal(cal_mask);
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
    
    if(adc_val < ref_0150mV)//0V
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
    else if(adc_val > ref_3150mV)//3.3V
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
                if((adc_val > ref_0150mV) && (adc_val < (adc_res_ratio[i] + adc_res_ratio[i + 1])*(ref_high - ref_low)/2))
                    break;
            }
            else if(i == adc_key_num - 1)
            {
                if((adc_val < ref_3150mV) && (adc_val > (adc_res_ratio[i] + adc_res_ratio[i - 1])*(ref_high - ref_low)/2))
                    break;
            }
            else
            {
                if((adc_val > (adc_res_ratio[i] + adc_res_ratio[i - 1])*(ref_high - ref_low)/2) && (adc_val < (adc_res_ratio[i] + adc_res_ratio[i + 1])*(ref_high - ref_low)/2))
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