#define C_TIMER_MUX (*(volatile unsigned *)0xc1109940)
#define C_TIMER_E (*(volatile unsigned *)0xc1109954)

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
    val |= (sel<<8);
    C_TIMER_MUX = val;
}

static void delay_us(unsigned us)
{
    C_TIMER_E = 0;
    while(C_TIMER_E < us);
}

static void delay_us(unsigned us)
{
    unsigned val, b_val;
    
    b_val = ((C_TIMER_MUX>>8) & 3);
    
    val = C_TIMER_MUX;
    val &= ~(3<<8);
    val |= (1<<8);
    C_TIMER_MUX = val;
    
    C_TIMER_E = 0;
    while(C_TIMER_E < us);
    
    val &= ~(3<<8);
    val |= (b_val<<8);
    C_TIMER_MUX = val;
}

static void delay_10us(unsigned n)
{
    unsigned val, b_val;
    
    b_val = ((C_TIMER_MUX>>8) & 3);
    
    val = C_TIMER_MUX;
    val &= ~(3<<8);
    val |= (2<<8);
    C_TIMER_MUX = val;
    
    C_TIMER_E = 0;
    while(C_TIMER_E < n);
    
    val &= ~(3<<8);
    val |= (b_val<<8);
    C_TIMER_MUX = val;
}

static void delay_100us(unsigned n)
{
    unsigned val, b_val;
    
    b_val = ((C_TIMER_MUX>>8) & 3);
    
    val = C_TIMER_MUX;
    val &= ~(3<<8);
    val |= (3<<8);
    C_TIMER_MUX = val;
    
    C_TIMER_E = 0;
    while(C_TIMER_E < n);
    
    val &= ~(3<<8);
    val |= (b_val<<8);
    C_TIMER_MUX = val;
}

static void delay_ms(unsigned ms)
{
    unsigned val;
    
    val = ((C_TIMER_MUX>>8) & 3);
    if(val == 1)
        ms = ms * 1000;
    else if(val == 2)
        ms = ms * 100;
    else if(val == 3)
        ms = ms * 10;
        
    C_TIMER_E = 0;
    while(C_TIMER_E < ms);
}