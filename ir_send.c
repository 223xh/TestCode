#define C_TIMER_MUX (*(volatile unsigned *)0x01000530)
#define C_TIMER_E (*(volatile unsigned *)0x01000544)

/**********************
0:	System clock 
1:	1uS
2:	10uS
3:	100uS
**********************/
static void set_timer_e_clock(unsigned sel)
{
    unsigned val;
    
    sel = sel>3?3:sel;
    val = C_TIMER_MUX;
    val &= ~(3<<8);
    val |= (1<<8);
    C_TIMER_MUX = val;
}

static void delay_us(unsigned us)
{
    C_TIMER_E = 0;
    while(C_TIMER_E < us);
}

static void set_ir_pwm(void)
{
    *(volatile unsigned *)0x12000b8 |= (1<<5);
}

static void clear_ir_pwm(void)
{
    *(volatile unsigned *)0x12000b8 &= ~(1<<5);
}

static void init_pwm(void)
{
    *(volatile unsigned *)0x12000b8 &= ~(0xf<<5);
    *(volatile unsigned *)0x12000b8 |= (1<<5);    
    
    *(volatile unsigned *)0x12000d0 = 0x013b013b;
    *(volatile unsigned *)0x12000d8 |= (1<<0);
    
    clear_ir_pwm();
}

static void ir_send_bit(unsigned char bit)
{    
    set_ir_pwm();
    delay_us(502);
    clear_ir_pwm();
    delay_us(502);
    if(bit)
        delay_us(1004);
}

static void ir_send_data(unsigned char data)
{
    unsigned char s_data, i;
    
    set_ir_pwm();
    delay_us(8028);
    clear_ir_pwm();
    delay_us(4014);
    
    for(i=0;i<8;i++)
        ir_send_bit(0);
    
    for(i=0;i<8;i++)
        ir_send_bit(1);
    
    s_data = (~data) & 0xff;
    for(i=0;i<8;i++)
    {
        if((s_data>>i) & 1)
            ir_send_bit(1);
        else
            ir_send_bit(0);
    }
    
    s_data = data;
    for(i=0;i<8;i++)
    {
        if((s_data>>i) & 1)
            ir_send_bit(1);
        else
            ir_send_bit(0);
    }
    
    ir_send_bit(0);
}