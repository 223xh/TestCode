// Define register address pointers

#define P_RTC_ADDR0                 (volatile unsigned long *)0xc1108740
#define P_RTC_ADDR1                 (volatile unsigned long *)0xc1108744
#define P_RTC_ADDR2                 (volatile unsigned long *)0xc1108748
#define P_RTC_ADDR3                 (volatile unsigned long *)0xc110874c
#define P_RTC_ADDR4                 (volatile unsigned long *)0xc1108750

#define P_ISA_TIMER_MUX             (volatile unsigned long *)0xc1109940
#define P_ISA_TIMER_E               (volatile unsigned long *)0xc1109954

// Define register RTC_ADDR0 bit map
#define RTC_REG0_BIT_sclk_static    20
#define RTC_REG0_BIT_ildo_ctrl_1    7
#define RTC_REG0_BIT_ildo_ctrl_0    6
#define RTC_REG0_BIT_test_mode      5
#define RTC_REG0_BIT_test_clk       4
#define RTC_REG0_BIT_test_bypass    3
#define RTC_REG0_BIT_sdi            2
#define RTC_REG0_BIT_sen            1
#define RTC_REG0_BIT_sclk           0

// Define register RTC_ADDR1 bit map
#define RTC_REG1_BIT_gpo_to_dig     3
#define RTC_REG1_BIT_gpi_to_dig     2
#define RTC_REG1_BIT_s_ready        1
#define RTC_REG1_BIT_sdo            0

// Define register RTC_ADDR3 bit map
#define RTC_REG3_BIT_count_always   17

// Define RTC serial protocal
#define RTC_SER_DATA_BITS           32
#define RTC_SER_ADDR_BITS           3

#define RTC_sbus_LOW(x)             ((*P_RTC_ADDR0) &= ~((1<<RTC_REG0_BIT_sen)|(1<<RTC_REG0_BIT_sclk)|(1<<RTC_REG0_BIT_sdi)))
#define RTC_sdi_HIGH(x)             ((*P_RTC_ADDR0) |=  (1<<RTC_REG0_BIT_sdi) )
#define RTC_sdi_LOW(x)              ((*P_RTC_ADDR0) &= ~(1<<RTC_REG0_BIT_sdi) )
#define RTC_sen_HIGH(x)             ((*P_RTC_ADDR0) |=  (1<<RTC_REG0_BIT_sen) )
#define RTC_sen_LOW(x)              ((*P_RTC_ADDR0) &= ~(1<<RTC_REG0_BIT_sen) )
#define RTC_sclk_HIGH(x)            ((*P_RTC_ADDR0) |=  (1<<RTC_REG0_BIT_sclk))
#define RTC_sclk_LOW(x)             ((*P_RTC_ADDR0) &= ~(1<<RTC_REG0_BIT_sclk))
#define RTC_sdo_READBIT             ((*P_RTC_ADDR1)&(1<<RTC_REG1_BIT_sdo))
#define RTC_sclk_static_HIGH(x)     ((*P_RTC_ADDR0) |=  (1<<RTC_REG0_BIT_sclk_static))
#define RTC_sclk_static_LOW(x)      ((*P_RTC_ADDR0) &= ~(1<<RTC_REG0_BIT_sclk_static))
#define RTC_count_always_HIGH(x)     ((*P_RTC_ADDR3) |= (1<<RTC_REG3_BIT_count_always))
#define RTC_count_always_LOW(x)      ((*P_RTC_ADDR3) &= ~(1<<RTC_REG3_BIT_count_always))

#define RTC_SER_REG_DATA_NOTIFIER   0xb41b

// Define RTC register address mapping
#define RTC_COUNTER_ADDR            0
#define RTC_GPO_COUNTER_ADDR        1
#define RTC_SEC_ADJUST_ADDR         2
#define RTC_UNUSED_ADDR_0           3
#define RTC_REGMEM_ADDR_0           4
#define RTC_REGMEM_ADDR_1           5
#define RTC_REGMEM_ADDR_2           6
#define RTC_REGMEM_ADDR_3           7

#define Wr(addr, data) *(volatile unsigned long *)(0xc1100000|((addr)<<2))=(data)

void delay_us1(unsigned us)
{
    unsigned val, b_val;
    
    b_val = ((*P_ISA_TIMER_MUX>>8) & 7);
    
    val = *P_ISA_TIMER_MUX;
    val &= ~(7<<8);
    val |= (1<<8);
    *P_ISA_TIMER_MUX = val;
    
    *P_ISA_TIMER_E = 0;
    while(*P_ISA_TIMER_E < us);
    
    val &= ~(7<<8);
    val |= (b_val<<8);
    *P_ISA_TIMER_MUX = val;
}

static void rtc_comm_delay(void)
{
    delay_us1(5);
}

void rtc_sclk_pulse(void)
{
    rtc_comm_delay();
    RTC_sclk_HIGH(1);
    rtc_comm_delay();
    RTC_sclk_LOW(0);
}

void rtc_wait_s_ready(void)
{
    while (!((*P_RTC_ADDR1)&(1<<RTC_REG1_BIT_s_ready)));
}

void rtc_comm_init(void)
{
    RTC_sbus_LOW(0);
    rtc_wait_s_ready();
    RTC_sen_HIGH(1);
}

void rtc_send_bit(unsigned val)
{
    if (val)
        RTC_sdi_HIGH(1);
    else
        RTC_sdi_LOW(0);
    rtc_sclk_pulse();
}

void rtc_send_addr_data(unsigned type, unsigned val)
{
    unsigned cursor = (type? (1<<(RTC_SER_ADDR_BITS-1)) : (1<<(RTC_SER_DATA_BITS-1)));
        
    while(cursor)
    {
        rtc_send_bit(val&cursor);
        cursor >>= 1;
    }
}

void rtc_get_data(unsigned *val)
{
    int i;
    for (i=0; i<RTC_SER_DATA_BITS; i++)
    {
        rtc_sclk_pulse();
        *val <<= 1;
        *val  |= RTC_sdo_READBIT;
    }
}

void rtc_set_mode(unsigned mode)
{
    RTC_sen_LOW(0);
    if (mode)
        RTC_sdi_HIGH(1);//WRITE
    else
        RTC_sdi_LOW(0);  //READ
    rtc_sclk_pulse();
    RTC_sdi_LOW(0);
}

unsigned ser_access_read(unsigned addr)
{
    unsigned val = 0;
    
    rtc_comm_init();
    rtc_send_addr_data(1,addr);
    rtc_set_mode(0); //Read
    rtc_get_data(&val);
    printf("ser_access_read: addr=0x%01x data=%u\n", addr, val);
    return val;
}

void ser_access_read_check(unsigned addr, unsigned exp_data, unsigned mask)
{
    unsigned act_data = ser_access_read(addr);
    
    if ((act_data | mask) != (exp_data | mask)) {
        printf("[Error] ser_access_read_check mismatch! addr=0x%01x exp_data=0x%08x act_data=0x%08x mask=0x%08x\n", addr, exp_data, act_data, mask);
    }
}

void ser_access_write(unsigned addr, unsigned data)
{
    printf("ser_access_write: addr=0x%01x data=%u\n", addr, data);
    rtc_comm_init();
    rtc_send_addr_data(0,data);
    rtc_send_addr_data(1,addr);
    rtc_set_mode(1); //Write
}

// -----------------------------------------------------------------------------
//                     Function: rtc_ser_static_write_manual
// Use part of the serial bus: sclk_static, sdi and sen to shift
// in a 16-bit static data. Manual mode.
// -----------------------------------------------------------------------------
void rtc_ser_static_write_manual (unsigned long static_reg_data_in)
{
    int i;
    
    printf("rtc_ser_static_write_manual: data=0x%04x\n", static_reg_data_in);

    // Initialize: sen low for 1 clock cycle
    RTC_sen_LOW(0);
    RTC_sclk_static_LOW(0);
    RTC_sclk_static_HIGH(1);
    RTC_sen_HIGH(1);
    RTC_sclk_static_LOW(0);
    // Shift in 16-bit known sequence
    for (i = 15; i >= 0; i --) {
        if ((RTC_SER_REG_DATA_NOTIFIER >> i) & 0x1) {
            RTC_sdi_HIGH(1);
        } else {
            RTC_sdi_LOW(0);
        }
        RTC_sclk_static_HIGH(1);
        RTC_sclk_static_LOW(0);
    }
    // 1 clock cycle turn around
    RTC_sdi_LOW(0);
    RTC_sclk_static_HIGH(1);
    RTC_sclk_static_LOW(0);
    // Shift in 16-bit static register data
    for (i = 15; i >= 0; i --) {
        if ((static_reg_data_in >> i) & 0x1) {
            RTC_sdi_HIGH(1);
        } else {
            RTC_sdi_LOW(0);
        }
        RTC_sclk_static_HIGH(1);
        RTC_sclk_static_LOW(0);
    }
    // One more clock cycle to complete write
    RTC_sen_LOW(0);
    RTC_sdi_LOW(0);
    RTC_sclk_static_HIGH(1);
    RTC_sclk_static_LOW(0);
} /* rtc_ser_static_write_manual */

void rtc_ser_static_write_auto (unsigned long static_reg_data_in)
{
    unsigned long data32;
    
    // Program MSB 15-8
    data32  = (static_reg_data_in >> 8) & 0xff;
    (*P_RTC_ADDR4) = data32;

    // Program LSB 7-0, and start serializing
    data32  = (*P_RTC_ADDR0);
    data32 |= 1                           << 17; // auto_serialize_start 
    data32 &= ~(0xff << 24);
    data32 |= (static_reg_data_in & 0xff) << 24; // auto_static_reg
    (*P_RTC_ADDR0) = data32;
    
    // Poll auto_serializer_busy bit until it's low (IDLE)
    while ((*P_RTC_ADDR0) & 1<<22) {}
} /* rtc_ser_static_write_auto */

void osc_clk_detect (void)

{
    unsigned osc_clk_count1;
    unsigned osc_clk_count2;

    delay_us1( 100 );

    RTC_count_always_HIGH(1);

    osc_clk_count1 = (*P_RTC_ADDR2);

    delay_us1( 50 );

    osc_clk_count2 = (*P_RTC_ADDR2);

    RTC_count_always_LOW(0);
    if( osc_clk_count1 == osc_clk_count2 ) {
        printf("[Error] osc_clk is NOT running: osc_clk_count1=osc_clk_count2=%d\n", osc_clk_count1);
    } else {
        printf("osc_clk is running: osc_clk_count1=%d osc_clk_count2=%d\n", osc_clk_count1, osc_clk_count2);
    }
} /* osc_clk_detect */