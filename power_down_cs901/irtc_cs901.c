#include "irtc_cs901.h"

static void rtc_comm_delay(void)
{
	for (int i=0;i<3;i++);
}

void delay_us(int us)
{
    (*C_TIMERE) = 0;
    while( (*C_TIMERE) < us ) {}
}

static int check_osc_clk(void)
{
	unsigned long   osc_clk_count1; 
	unsigned long   osc_clk_count2;   
	(*RTC_REG3)    |= (1 << 17);   // Enable count always    

	osc_clk_count1 = (*RTC_REG2);    // Wait for 50uS.  32.768khz is 30.5uS.  This should be long   
								  // enough for one full cycle of 32.768 khz   
	delay_us( 50 );   
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

static void rtc_send_bit(unsigned val)
{
	if (val)
		RTC_sdi_HIGH(1);
	else
		RTC_sdi_LOW(0);
	rtc_sclk_pulse();
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

static unsigned ser_access_4b_cs (unsigned  wr_mode, unsigned  addr,
                             unsigned  byte_sel, unsigned  wr_data)
{
    unsigned  sdo = 0;
    unsigned  rd_data = 0;
    int i;
    
    // -----------------------------------------------
    // Emulate serial bus activity
    //
    // Assert sen
    RTC_sen_HIGH(1);
    // Wr_data phase (Write access only!)
    if (wr_mode) {
        for (i = RTC_SER_DATA_BITS - 1; i >= 0; i--) {
            rtc_send_bit((wr_data >> i) & 0x1);

        }
    }
    // Addr phase
    for (i = RTC_SER_ADDR_BITS - 1; i >= 0; i --) {
 				rtc_send_bit((addr >> i) & 0x1);
    }
    // Byte_sel phase
    for (i = 3 - 1; i >= 0; i --) {
 				rtc_send_bit((byte_sel >> i) & 0x1);
    }
    
    rtc_set_mode(wr_mode);

    // Rd_data phase (Read access only!)
    if (! wr_mode) {
    	
    	rd_data = 0;
        // For reading byte0 only: need to poll s_ready before reading data out
        if (byte_sel == 0) {
            rtc_wait_s_ready();
        }
        for (i = RTC_SER_DATA_BITS - 1; i >= 0; i--) {
						
						rtc_sclk_pulse();
						rd_data <<= 1;
						rd_data  |= RTC_Sdo_READBIT;
        }
    }
    
    
    return(rd_data);
}

static unsigned ser_read_cs (unsigned addr)
{
    unsigned  rd_data = 0;
    for (int i = 0; i < 8; i ++) {
        rd_data |= ser_access_4b_cs(0, addr, i, 0) << (i*4);
    }
    return(rd_data);
} 

static int ser_write_cs (unsigned addr, unsigned wr_data)
{
    for (int i = 0; i < 8; i ++) {
        ser_access_4b_cs(1, addr, i, (wr_data >> (i*4)) & 0xf);

       rtc_wait_s_ready();
    }
    
    return 0;
}

unsigned irtc_read(unsigned addr)
{
	unsigned val = 0;
	
	if(check_osc_clk() < 0){
		return val;
	}
	val = ser_read_cs(addr);
	
	return val;
}

int irtc_write(unsigned addr, unsigned data)
{
	ser_write_cs(addr, data);

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