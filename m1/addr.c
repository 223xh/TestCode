static void ddr_addr_test()
{
    unsigned i, j = 0;
    
addr_start:

    reg(MEM_START_ADDR) = 0x55aaaa55;
    //for(i=2;i<28;i++)
    for(i=2;i<29;i++)
    {
        reg(MEM_START_ADDR+(1<<i)) = (i-1)<<(j*8);
    }

    serial_put_dword(MEM_START_ADDR);
    serial_puts(" = ");
    serial_put_dword(reg(MEM_START_ADDR));
    serial_puts("\n");
    //for(i=2;i<28;i++)
    for(i=2;i<29;i++)
    {
       serial_put_dword(MEM_START_ADDR+(1<<i));
       serial_puts(" = ");
       serial_put_dword(reg(MEM_START_ADDR+(1<<i)));
       serial_puts("\n");
    }
    
    if(j < 3){
        ++j;
        goto addr_start;
    }
}

static void ddr_addr_test()
{
    unsigned i;
    reg(MEM_START_ADDR) = 0x55aaaa55;
    //for(i=2;i<28;i++)
    for(i=2;i<29;i++)
    {
        reg(MEM_START_ADDR+(1<<i)) = (i-1) | ((i-1)<<16);
    }

    serial_put_dword(MEM_START_ADDR);
    serial_puts(" = ");
    serial_put_dword(reg(MEM_START_ADDR));
    serial_puts("\n");
    //for(i=2;i<28;i++)
    for(i=2;i<29;i++)
    {
        //r_addr[i-2][0] = MEM_START_ADDR+(1<<i);
        //r_addr[i-2][1] = reg(MEM_START_ADDR+(1<<i));
       serial_put_dword(MEM_START_ADDR+(1<<i));
       serial_puts(" = ");
       serial_put_dword(reg(MEM_START_ADDR+(1<<i)));
       serial_puts("\n");
    }
}