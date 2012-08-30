static void init_ir(void)
{
    wr_reg(0xc1200100, 0x01d801ac);
    wr_reg(0xc1200104, 0x00f800ca);
    wr_reg(0xc1200108, 0x007a0066);
    //wr_reg(0xc120010c, 0x0044002c);
    wr_reg(0xc120010c, 0x0044002e);
    //wr_reg(0xc1200110, 0x00fa0011 | (3<<28));
    wr_reg(0xc1200110, 0x00fa0013 | (3<<28));
    //wr_reg(0xc1200118, 0x08915d00);
    wr_reg(0xc1200118, 0x08915c00);
    //wr_reg(0xc120011c, 0x02213e70);
    wr_reg(0xc120011c, 0x00003e70);
    
    wr_reg(0xc12000b4, rd_reg(0xc12000b4) & (~(1<<28)));
    wr_reg(0xc12000b8, rd_reg(0xc12000b8) | (1<<17)); //enable remote
}

static void test_ir(void)
{
    unsigned tmp;
    
    init_ir();
    while(1)
    {
        if(rd_reg(0xc1200118) & (1<<3))
        {
            tmp = rd_reg(0xc1200114)>>24;
            printf("remote key value is[0x%x]\n",tmp);
        }
        AVTimeDly(40);
    }
}