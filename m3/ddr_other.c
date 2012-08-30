//pctl:
unsigned short zqcr = 0;

if(zqcr)
            APB_Wr(PCTL_ZQCR_ADDR, (1 << 24) | zqcr);
         else
         
//ddr.c:

void set_ddr_clock(struct ddr_set * timing_reg)
{
 int i;
#ifdef ENABLE_POWER_SAVING
    APB_Wr(PCTL_DLLCR_ADDR, APB_Rd(PCTL_DLLCR_ADDR)|(7<<2));
#endif

#ifdef CONFIG_AML_DDR_PLL_FINE_TUNE
    zqcr = Rd(PREG_STICKY_REG0) >> 16;

    if((Rd(PREG_STICKY_REG0) & 0xffff) == 0x2012){
        Wr(HHI_DDR_PLL_CNTL, Rd(PREG_STICKY_REG1) |0x8000);
    }
    else{
        Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl |0x8000);
    }

    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);

    if((Rd(PREG_STICKY_REG0) & 0xffff) == 0x2012){
        Wr(HHI_DDR_PLL_CNTL, Rd(PREG_STICKY_REG1));
        Wr(PREG_STICKY_REG0, 0);
        Wr(PREG_STICKY_REG1, 0);
    }
    else{
        Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl);
    }

    __udelay(10);
	while(!(readl(P_HHI_DDR_PLL_CNTL3)&(1<<31)));
#else
    Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl |0x8000);
    Wr(HHI_DDR_PLL_CNTL2,0x65e31ff);
    Wr(HHI_DDR_PLL_CNTL3,0x1649a941);
    Wr(HHI_DDR_PLL_CNTL, timing_reg->ddr_pll_cntl);
	while(!(readl(P_HHI_DDR_PLL_CNTL3)&(1<<31)));
    wait_pll(3,timing_reg->ddr_clk);
    wait_pll(30,timing_reg->ddr_clk);
#endif

    Wr(RESET1_REGISTER,1<<3);
    __udelay(1000);
//    for ( i = 0; i <= 1000; i ++) {
//       APB_Wr(MMC_DDR_CTRL, timing_reg->ddr_ctrl );   //
//   }
#ifndef CONFIG_AML_DDR_PLL_FINE_TUNE
    serial_puts("\nddr3 : ");
    serial_put_hex(timing_reg->ddr_clk,32);
    serial_puts(" MHz\n");
#endif
}