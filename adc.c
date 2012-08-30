#include "adc.h"

typedef unsigned char bool;
#define true 1
#define false 0

void adc_init(unsigned ch)
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
}

bool adc_check(unsigned threshold_high, unsigned threshold_low)
{
	unsigned data = 0;
	
	ADC_stop_sample;
	// Wait for ADC sampling fully stopped
	while ((*ADC_TSCREEN_REG0)&0x70000000);
	// Empty FIFO
	while (((*ADC_TSCREEN_REG0)>>21)&0x1f)
		data = (*ADC_TSCREEN_FIFO_RD)&0x3ff;
	ADC_start_sample;
	// Wait for new sample finished
	while (!(((*ADC_TSCREEN_REG0)>>21)&0x1f));
	// Read the 1st sample data
	data = (*ADC_TSCREEN_FIFO_RD)&0x3ff;
	if (data > threshold_high)
		return false;
	if (data < threshold_low)
		return false;
	return true;
}