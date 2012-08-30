#define     UART_WR_FIFO            (volatile unsigned long *)0x01200120
#define     UART_RD_FIFO            (volatile unsigned long *)0x01200124
#define     UART_REG2               (volatile unsigned long *)0x01200128
#define     UART_STATUS             (volatile unsigned long *)0x0120012c
#define     UART_IRQ                (volatile unsigned long *)0x01200130

#define UART_BAUD 115200
void init_uart(void)
{  
    unsigned sys_clk, baud_rate, ret;    
    sys_clk = am_get_system_clock();
    
    *(volatile unsigned *)0x012000b8 |= 1<<15;   //switch IOMUX0(RX) to UART MODE
    *(volatile unsigned *)0x012000b8 |= 1<<10;   //switch IOMUX1(TX) to UART MODE
    
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