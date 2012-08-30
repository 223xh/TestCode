#include "irtc.h"

void delay_us1(int us)
{
    (*C_TIMERE) = 0;
    while( (*C_TIMERE) < us ) {}
}

static void rtc_comm_delay(void)
{
    delay_us1(5);
}

static int check_osc_clk(void)
{
	unsigned long   osc_clk_count1; 
	unsigned long   osc_clk_count2;   
	(*RTC_REG3)    |= (1 << 17);   // Enable count always    

	osc_clk_count1 = (*RTC_REG2);    // Wait for 50uS.  32.768khz is 30.5uS.  This should be long   
								  // enough for one full cycle of 32.768 khz   
	delay_us1( 50 );   
	osc_clk_count2 = (*RTC_REG2);    

	(*RTC_REG3)    &= ~(1 << 17);  // disable count always    

	if( osc_clk_count1 == osc_clk_count2 ) { 
	//PRINT("The osc_clk is not running now! need to invcrease the power!\n");
	return(-1); 
	}

	return(0);

}

static void rtc_sclk_pulse(void)
{
	rtc_comm_delay();
	RTC_sclk_HIGH(1);
	rtc_comm_delay();
	RTC_sclk_LOW(0);
}

static int rtc_wait_s_ready(void)
{
	int i = 100000;
	while (i--){
		if((*(volatile unsigned *)RTC_ADDR1)&s_ready)
			break;
		}
	return i;
}

static int rtc_comm_init(void)
{
	RTC_sbus_LOW(0);
	if(rtc_wait_s_ready()>0){
		RTC_sen_HIGH(1);
		return 0;
	}
	return -1;
}

static void rtc_send_bit(unsigned val)
{
	if (val)
		RTC_sdi_HIGH(1);
	else
		RTC_sdi_LOW(0);
	rtc_sclk_pulse();
}

static void rtc_send_addr_data(unsigned type, unsigned val)
{
	unsigned cursor = (type? (1<<(RTC_SER_ADDR_BITS-1)) : (1<<(RTC_SER_DATA_BITS-1)));
		
	while(cursor)
	{
		rtc_send_bit(val&cursor);
		cursor >>= 1;
	}
}

static void rtc_get_data(unsigned *val)
{
	for (unsigned i=0; i<RTC_SER_DATA_BITS; i++)
	{
		rtc_sclk_pulse();
		*val <<= 1;
		*val  |= RTC_Sdo_READBIT;
	}
}

static void rtc_set_mode(unsigned mode)
{
	RTC_sen_LOW(0);
	if (mode)
		RTC_sdi_HIGH (1);//WRITE
	else
		RTC_sdi_LOW(0);  //READ
	rtc_sclk_pulse();
	RTC_sdi_LOW(0);
}

unsigned irtc_read(unsigned addr)
{
	unsigned val = 0;
	
//	if(check_osc_clk() < 0)
//		return 0;	

	if(rtc_comm_init() < 0)
		return 0;
		
	rtc_send_addr_data(1,addr);
	rtc_set_mode(0); //Read
	rtc_get_data(&val);
	
	return val;
}

int irtc_write(unsigned addr, unsigned data)
{
	if(rtc_comm_init() < 0)
		return -1;
	rtc_send_addr_data(0,data);
	rtc_send_addr_data(1,addr);
	rtc_set_mode(1); //Write
	
	rtc_wait_s_ready();

	return 0;
}

static unsigned get_bit(unsigned  reg_addr, unsigned bit_num)
{
	unsigned data = 0;
	data = *((volatile unsigned *)reg_addr);
	data >>= bit_num;
	data &= 1;

	return data;
}

void static_reg_write(unsigned data)
{
	unsigned reg_data = 0;
	reg_data = *(RTC_REG0);
	reg_data |= 1 << 17;
	reg_data &= ~(0xff<<24);
	reg_data |= (data & 0xff ) << 24;

	*(RTC_REG0) = reg_data;

	while(get_bit(RTC_ADDR0, 22)){}
	
}

static void rtc_sclk_pulse_ss(void)
{
	rtc_comm_delay();
	RTC_sclk_HIGH_ss(1);
	rtc_comm_delay();
	RTC_sclk_LOW_ss(0);
}

static void rtc_comm_init_ss(void)
{
    RTC_sbus_LOW_ss(0);
    
    rtc_sclk_pulse_ss();
    RTC_sen_HIGH_ss(1);
}

static void rtc_send_bit_ss(unsigned val)
{
	if(val)
		RTC_sdi_HIGH_ss(1);
	else
		RTC_sdi_LOW_ss(0);
	rtc_sclk_pulse_ss();
}

static void rtc_send_addr_data_ss(unsigned type, unsigned val)
{
	unsigned cursor = (type? (1<<(RTC_UNIQUE_CODE_BITS-1)) : (1<<(RTC_STATIC_DATA_BITS-1)));
		
	while(cursor)
	{
		rtc_send_bit_ss(val&cursor);
		cursor >>= 1;
	}
}

static void rtc_one_cycle_ss(unsigned char flag)
{
    RTC_sdi_LOW_ss(0);
    if(flag)
        RTC_sen_LOW_ss(0);
    rtc_sclk_pulse_ss();
}

int irtc_write_ss(unsigned data)
{
    unsigned hi = 0xb41b025b;
    unsigned lo = 0xd20033e9;

    rtc_comm_init_ss();
    rtc_send_addr_data_ss(1, hi);
    rtc_send_addr_data_ss(1, lo);
    rtc_one_cycle_ss(0);
    rtc_send_addr_data_ss(0, data);
    rtc_one_cycle_ss(1);
    
    rtc_wait_s_ready();
    
    return 0;
}