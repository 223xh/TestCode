static unsigned get32bit(void)
{
    unsigned data;
    int indata, outdata, i;

    for(i=0;i<8;i++){
        indata = serial_getc();
        if((indata >= '0') && (indata<='9')){
            outdata |= ((indata - '0') & 0xf)<<(i*4);
            serial_putc(indata);
        }
        else if((indata >= 'a') && (indata<='f')){
            outdata |= ((indata - 'a' + 10) & 0xf)<<(i*4);
            serial_putc(indata);
        }
        else if((indata >= 'A') && (indata<='Z')){
            outdata |= ((indata - 'a' + 10) & 0xf)<<(i*4);
            serial_putc(indata);
        }
        else{
            --i;
        }
    }

    while(indata != '\n'){
         indata = serial_getc();
    }

    return outdata;
}

static void set_result()
{
    unsigned rslr0, rdgr0, dllcr[4], dqstr, dqsntr, dqtr[4];

    serial_puts("\nInput RSLR0:");
    rslr0 = get32bit();
    serial_puts("\nInput RDGR0:");
    rdgr0 = get32bit();

    serial_puts("\nInput DLLCR0:");
    dllcr[0] = get32bit();
    serial_puts("\nInput DLLCR1:");
    dllcr[1] = get32bit();
    serial_puts("\nInput DLLCR2:");
    dllcr[2] = get32bit();
    serial_puts("\nInput DLLCR3:");
    dllcr[3] = get32bit();

    serial_puts("\nInput DQSTR:");
    dqstr = get32bit();
    serial_puts("\nInput DQSNTR:");
    dqsntr = get32bit();

    serial_puts("\nInput DQTR0:");
    dqtr[0] = get32bit();
    serial_puts("\nInput DQTR1:");
    dqtr[1] = get32bit();
    serial_puts("\nInput DQTR2:");
    dqtr[2] = get32bit();
    serial_puts("\nInput DQTR3:");
    dqtr[3] = get32bit();

    APB_Wr(PCTL_RSLR0_ADDR,rslr0);
    APB_Wr(PCTL_RDGR0_ADDR,rdgr0);

    APB_Wr(PCTL_DLLCR0_ADDR,dllcr[0]);
    APB_Wr(PCTL_DLLCR1_ADDR,dllcr[1]);
    APB_Wr(PCTL_DLLCR2_ADDR,dllcr[2]);
    APB_Wr(PCTL_DLLCR3_ADDR,dllcr[3]);

    APB_Wr(PCTL_DQSTR_ADDR,dqstr);
    APB_Wr(PCTL_DQSNTR_ADDR,dqsntr);

    APB_Wr(PCTL_DQTR0_ADDR,dqtr[0]);
    APB_Wr(PCTL_DQTR1_ADDR,dqtr[1]);
    APB_Wr(PCTL_DQTR2_ADDR,dqtr[2]);
    APB_Wr(PCTL_DQTR3_ADDR,dqtr[3]);

    serial_puts("\nRSLR0=");
    serial_put_hex(APB_Rd(PCTL_RSLR0_ADDR),32);
    serial_puts("\nRDGR0=");
    serial_put_hex(APB_Rd(PCTL_RDGR0_ADDR),32);

    serial_puts("\nDLLCR0=");
    serial_put_hex(APB_Rd(PCTL_DLLCR0_ADDR),32);
    serial_puts("\nDLLCR1=");
    serial_put_hex(APB_Rd(PCTL_DLLCR1_ADDR),32);
    serial_puts("\nDLLCR2=");
    serial_put_hex(APB_Rd(PCTL_DLLCR2_ADDR),32);
    serial_puts("\nDLLCR3=");
    serial_put_hex(APB_Rd(PCTL_DLLCR3_ADDR),32);

    serial_puts("\nDQSTR=");
    serial_put_hex(APB_Rd(PCTL_DQSTR_ADDR),32);
    serial_puts("\nDQSNTR=");
    serial_put_hex(APB_Rd(PCTL_DQSNTR_ADDR),32);

    serial_puts("\nDQTR0=");
    serial_put_hex(APB_Rd(PCTL_DQTR0_ADDR),32);
    serial_puts("\nDQTR1=");
    serial_put_hex(APB_Rd(PCTL_DQTR1_ADDR),32);
    serial_puts("\nDQTR2=");
    serial_put_hex(APB_Rd(PCTL_DQTR2_ADDR),32);
    serial_puts("\nDQTR3=");
    serial_put_hex(APB_Rd(PCTL_DQTR3_ADDR),32);

}