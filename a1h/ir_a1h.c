static void init_ir(void)
{
    wr_reg(0xc1100000 | (0x2120<<2), 0x01d801ac);
    wr_reg(0xc1100000 | (0x2121<<2), 0x00f800ca);
    wr_reg(0xc1100000 | (0x2122<<2), 0x007a0066);
    wr_reg(0xc1100000 | (0x2123<<2), 0x0044002e);
    wr_reg(0xc1100000 | (0x2124<<2), 0x00fa0013 | (3<<28));
    wr_reg(0xc1100000 | (0x2126<<2), 0x08915c00);//f21
    wr_reg(0xc1100000 | (0x2127<<2), 0x0000be40);
    
    rd_reg(0xc1100000 | (0x202d<<2)) &= ~(1<<28);
    rd_reg(0xc1100000 | (0x202e<<2)) |= (1<<17); //enable remote
}

static void test_ir(void)
{
    unsigned tmp;
    
    init_ir();
    while(1)
    {
        if(rd_reg(0xc1100000 | (0x2126<<2)) & (1<<3))
        {
            tmp = rd_reg(0xc1100000 | (0x2125<<2))>>24;
            printf("remote key value is[0x%x]\n",tmp);
        }
        AVTimeDly(40);
    }
}