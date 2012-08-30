#ifndef ADC_H
#define ADC_H

/************************
NIKED3 Only
void adc_init(ch):
Pin_name      ch
TADC_CH11   7
TADC_CH10   6
TADC_CH9     0
TADC_CH8     1
TADC_CH7     3
TADC_CH6     5
TADC_CH5     2
TADC_CH4     4
************************/
#define TADC_CH11   7
#define TADC_CH10   6
#define TADC_CH9    0
#define TADC_CH8    1
#define TADC_CH7    3
#define TADC_CH6    5
#define TADC_CH5    2
#define TADC_CH4    4

#define     ADC_TSCREEN_REG0              (volatile unsigned long *)0x01200380
#define     ADC_TSCREEN_CHAN_LIST         (volatile unsigned long *)0x01200384
#define     ADC_TSCREEN_AVG_CNTL          (volatile unsigned long *)0x01200388
#define     ADC_TSCREEN_REG3              (volatile unsigned long *)0x0120038c
#define     ADC_TSCREEN_DELAY             (volatile unsigned long *)0x01200390
#define     ADC_TSCREEN_LAST_RD           (volatile unsigned long *)0x01200394
#define     ADC_TSCREEN_FIFO_RD           (volatile unsigned long *)0x01200398
#define     ADC_TSCREEN_AUX_SW            (volatile unsigned long *)0x0120039c
#define     ADC_TSCREEN_CHAN_10_SW        (volatile unsigned long *)0x012003a0
#define     ADC_TSCREEN_DETECT_IDLE_SW    (volatile unsigned long *)0x012003a4
#define     ADC_TSCREEN_DELTA_10          (volatile unsigned long *)0x012003a8
#define     ADC_Band_Gap_Control          (volatile unsigned long *)0x01200294

#define     ADC_stop_sample               ( *ADC_TSCREEN_REG0     |= (1 << 14))
#define     ADC_start_sample              ( *ADC_TSCREEN_REG0     |= (1 <<  2))
#define     ADC_enable_sample             ( *ADC_TSCREEN_REG0     |= (1 <<  0))
#define     ADC_enable_band_gap           ( *ADC_Band_Gap_Control |= (1 <<  0)) // For Nike-D3

#endif