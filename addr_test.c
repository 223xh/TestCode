static void ddr_addr_test()
{
    unsigned i, j = 0;

addr_start:
    serial_puts("********************\n");
    writel(0x55aaaa55, PHYS_MEMORY_START);
    for(i=2;(((1<<i)&PHYS_MEMORY_SIZE) == 0) ;i++)
    {
        writel((i-1)<<(j*8), PHYS_MEMORY_START+(1<<i));
    }

    serial_put_hex(PHYS_MEMORY_START, 32);
    serial_puts(" = ");
    serial_put_hex(readl(PHYS_MEMORY_START), 32);
    serial_puts("\n");
    for(i=2;(((1<<i)&PHYS_MEMORY_SIZE) == 0) ;i++)
    {
       serial_put_hex(PHYS_MEMORY_START+(1<<i), 32);
       serial_puts(" = ");
       serial_put_hex(readl(PHYS_MEMORY_START+(1<<i)), 32);
       serial_puts("\n");
    }

    if(j < 3){
        ++j;
        goto addr_start;
    }
}