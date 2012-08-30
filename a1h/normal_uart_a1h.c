#define     UART_WR_FIFO            (volatile unsigned long *)(0xc1100000 | (0x2130<<2))
#define     UART_RD_FIFO            (volatile unsigned long *)(0xc1100000 | (0x2131<<2))
#define     UART_REG2               (volatile unsigned long *)(0xc1100000 | (0x2132<<2))
#define     UART_STATUS             (volatile unsigned long *)(0xc1100000 | (0x2133<<2))
#define     UART_IRQ                (volatile unsigned long *)(0xc1100000 | (0x2134<<2))

#define UART_BAUD 115200

void Update_UART_CLK()
{
    unsigned baud_rate, sys_clk;
    sys_clk = (My_Get_MPEG_CLK()>>1) * 10;
    baud_rate = (sys_clk * 1000) / (UART_BAUD / 100 * 4) - 1;
    (*UART_REG2) = 0x18003000 | baud_rate;
}

void init_uart()
{  
    unsigned sys_clk, baud_rate, ret;    
    sys_clk = get_sysclk();

    *(volatile unsigned *)(0xc11080b0) &= ~(0xf<<9);
    *(volatile unsigned *)(0xc11080b0) |= 1<<14;  //switch GPIOB_17 IOMUXB_0(RX) to UART MODE
    *(volatile unsigned *)(0xc11080b0) |= 1<<10;  //switch GPIOB_18 IOMUXB_1(TX) to UART MODE
#if 0
    //*(volatile unsigned *)(0xc1100000 | (0x202c<<2)) |= 1<<6;   //switch GPIOB_19 IOMUXC_0(RX) to UART MODE
    *(volatile unsigned *)(0xc1100000 | (0x202c<<2)) |= 1<<2;   //switch GPIOB_20 IOMUXC_1(TX) to UART MODE
//#else
    *(volatile unsigned *)(0xc11080cc) |= 1<<13;  //switch GPIOD_0 IOMUXD_0(RX) to UART MODE
    *(volatile unsigned *)(0xc11080cc) |= 1<<9;   //switch GPIOD_1 IOMUXD_1(TX) to UART MODE
#endif
    
    //(*UART_REG2) &= ~(1 << 19); // Disable parity
    
    //(*UART_REG2) = ((*UART_REG2) & ~(0xFFF << 0)) | 285;   // 132Mhz/(115200*4)    (115200 Baud)
    
    //(*UART_REG2) = 0x1800311d;
    (*UART_REG2) = 0x18003000;
    baud_rate = (sys_clk * 1000) / (UART_BAUD / 100 * 4) - 1;
    (*UART_REG2) |= baud_rate;
    (*UART_IRQ) = 0x81;
    
    // Reset the UART
    (*UART_REG2) |=  ( (1 << 22) | (1 << 23) | (1 << 24) );
    (*UART_REG2) &= ~( (1 << 22) | (1 << 23) | (1 << 24) );

    AVTimeDly(100);
}

void WriteUart(char * wbuff, unsigned len)
{
    unsigned i;
    for(i = 0; i < len; i++)
    {
        if(!(*UART_STATUS & 0x200000))
            (*UART_WR_FIFO) = wbuff[i];
        //AVTimeDly(1);
    }
}

unsigned char Check_Buffer_LEN()
{
    unsigned char n;
    n = (*UART_STATUS) & 0x7f;
    return n;
}

int ReadUart(char * rbuff, unsigned len, unsigned retry)
{
    unsigned r_len = 0;
    unsigned n = 0;
    while((retry--) && (r_len<len))
    {
        n = Check_Buffer_LEN();
        if(n)
        {
            if(r_len + n > len)
            {
                n = len - r_len;
                r_len = len;
            }
            else
                r_len += n;
                
            do
            {
                *rbuff = (*UART_RD_FIFO) & 0xff;
                ++rbuff;
            }
            while(--n);
        }
    }
    if(retry)
        return 0;
    else
        return -1;
}