#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))

static void get_num(unsigned num, unsigned char *n)
{
    *n = 0;
    *(n+1) = 0;
    *(n+2) = 0;
    *(n+3) = 0;

    while(num>=1000){
        ++(*(n+3));
        num -= 1000;
    }

    while(num>=100){
        ++(*(n+2));
        num -= 100;
    }

    while(num>=10){
        ++(*(n+1));
        num -= 10;
    }

    *n = num;
}

void show_clk(unsigned clk)
{
    unsigned char c[4];

    get_num(clk , c);
    serial_putc('0'+ c[3]);
    serial_putc('0'+ c[2]);
    serial_putc('0'+ c[1]);
    serial_putc('0'+ c[0]);
}

    if(creg(0x207c) == 0xb2011){
        writel(creg(0x207d)|0x8000,P_HHI_SYS_PLL_CNTL);//800Mhz(0x664),600Mhz,400Mhz , 200Mhz
    }
    else{
        writel(plls->sys_pll_cntl|0x8000,P_HHI_SYS_PLL_CNTL);//800Mhz(0x664),600Mhz,400Mhz , 200Mhz
    }

    writel(0x65e31ff  ,P_HHI_SYS_PLL_CNTL2);
    writel(0x1649a941 , P_HHI_SYS_PLL_CNTL3);

    if(creg(0x207c) == 0xb2011){
        writel(creg(0x207d)&(~0x8000),P_HHI_SYS_PLL_CNTL);
        plls->sys_clk = ((creg(0x207d) & 0x1ff)*24/((creg(0x207d)>>9) & 0x1f))>>((creg(0x207d)>>16) & 0x3) ;
        creg(0x207c) = 0;
        creg(0x207d) = 0;
    }
    else{
        writel(plls->sys_pll_cntl&(~0x8000),P_HHI_SYS_PLL_CNTL);
    }
    
    
    
    if(creg(0x207c) == 0xa2011){
        Wr(HHI_DDR_PLL_CNTL, creg(0x207d) |0x8000);
    }
    else{
        Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl |0x8000);
    }

    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);

    if(creg(0x207c) == 0xa2011){
        Wr(HHI_DDR_PLL_CNTL, creg(0x207d));
        creg(0x207c) = 0;
        creg(0x207d) = 0;
    }
    else{
        Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl);
    }