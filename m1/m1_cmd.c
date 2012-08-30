#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))

#define delay_us(a) udelay(a)

static void wifi_sdio_test(unsigned long n)
{
    //int n;
    
    /***Clear pinmux***/    
    reg(0xc11080bc) &= ~((1<<16) | (1<<22));reg(0xc11080d4) &= ~(1<<0);reg(0xc11080d8) &= ~(1<<0);//J20 GPIO_C0 WIFI/BT_WAKE
    reg(0xc11080dc) &= ~(1<<6);reg(0xc11080b8) &= ~((1<<16) | (1<<20));reg(0xc11080c8) &= ~(1<<27);//W12 GPIO_B7 SD_D3_C
    reg(0xc11080dc) &= ~(1<<7);reg(0xc11080b8) &= ~((1<<17) | (1<<21));//Y12 GPIO_B6 SD_D2_C
    reg(0xc11080dc) &= ~(1<<8);reg(0xc11080b8) &= ~((1<<18) | (1<<22));//W11 GPIO_B5 SD_D1_C
    reg(0xc11080dc) &= ~(1<<9);reg(0xc11080b8) &= ~((1<<19) | (1<<23));//Y11 GPIO_B4 SD_D0_C
    reg(0xc11080b8) &= ~(0xf<<8);reg(0xc11080c8) &= ~(1<<30);//W10 GPIO_B3 SD_CLK_C
    reg(0xc11080b8) &= ~(0xf<<12);reg(0xc11080c8) &= ~(1<<31);//Y10 GPIO_B2 SD_CMD_C
    
    /***Pull low***/
    creg(0x2010) &= ~(1<<0);//GPIOC0
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    
    /***Output enable***/
    creg(0x200c) &= ~(0x3f<<21);//GPIOB2-B7
    creg(0x200f) &= ~(1<<0);//GPIOC0
    
    delay_us(10000);
    
    //n = 3;
    
pstart:
    /***Pull high***/
    creg(0x2010) |= (1<<0);//GPIOC0
    creg(0x200d) |= (0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull low***/
    creg(0x2010) &= ~(1<<0);//GPIOC0
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x3f<<21);//GPIOB2-B7
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x1f<<21);//GPIOB2-B6
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x1f<<21);//GPIOB2-B6
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0xf<<21);//GPIOB2-B5
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0xf<<21);//GPIOB2-B5
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x7<<21);//GPIOB2-B4
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x7<<21);//GPIOB2-B4
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x3<<21);//GPIOB2-B3
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x3<<21);//GPIOB2-B3
    delay_us(10000);
    /***Pull high***/
    creg(0x200d) |= (0x1<<21);//GPIOB2
    delay_us(10000);
    /***Pull low***/
    creg(0x200d) &= ~(0x1<<21);//GPIOB2
    delay_us(30000);
    
    if(--n)
        goto pstart;

}

static void i2c_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080b8) &= ~(0x3f<<0);reg(0xc11080c8) &= ~(3<<28);reg(0xc11080b4) &= ~(1<<31);//AA10 & AB10     GPIOB0(SCL) & GPIOB1(SDA)     HW_I2C_SDA & HW_I2C_SCL
    
    /***Pull low***/    
    creg(0x200d) &= ~(0x3<<19);//GPIOB0-B1
    
    /***Output enable***/
    creg(0x200c) &= ~(0x3<<19);//GPIOB0-B1
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x200d) |= (0x3<<19);//GPIOB0-B1
        delay_us(10000);
        /***Pull low***/
        creg(0x200d) &= ~(0x3<<19);//GPIOB0-B1
        delay_us(10000);
        /***Pull high***/
        creg(0x200d) |= (0x1<<19);//GPIOB0
        delay_us(10000);
        /***Pull low***/
        creg(0x200d) &= ~(0x1<<19);//GPIOB0
        delay_us(30000);    
    }while(--n);
}

static void ir_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080c8) &= ~(1<<9);reg(0xc11080c4) &= ~(1<<31);//A14 GPIOE21 IR
    
    /***Pull low***/    
    creg(0x2016) &= ~(1<<21);//GPIOE21
    
    /***Output enable***/
    creg(0x2015) &= ~(1<<21);//GPIOE21
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x2016) |= (1<<21);//GPIOE21
        delay_us(10000);
        /***Pull low***/
        creg(0x2016) &= ~(1<<21);//GPIOE21
        delay_us(30000);    
    }while(--n);
}

static void itu601_test(unsigned long n)
{    
    /***Clear pinmux***/    
    reg(0xc11080bc) &= ~(0x1ffffff<<0);reg(0xc11080b8) &= ~(0xf<<24);reg(0xc11080c0) &= ~(0x3<<26);reg(0xc11080d4) &= ~(0xfff<<1);reg(0xc11080d8) &= ~(0xfff<<1);//GPIO C1-C12
    
    /***Pull low***/    
    creg(0x2010) &= ~(0xfff<<1);//GPIO C1-C12
    
    /***Output enable***/
    creg(0x200f) &= ~(0xfff<<1);//GPIO C1-C12
    
    delay_us(10000);
    
    do{    
        /***Pull high***/
        creg(0x2010) |= (0xfff<<1);//GPIO C1-C12
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xfff<<1);//GPIO C1-C12
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7ff<<1);//GPIO C1-C11
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7ff<<1);//GPIO C1-C11
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3ff<<1);//GPIO C1-C10
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3ff<<1);//GPIO C1-C10
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x1ff<<1);//GPIO C1-C9
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x1ff<<1);//GPIO C1-C9
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0xff<<1);//GPIO C1-C8
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xff<<1);//GPIO C1-C8
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7f<<1);//GPIO C1-C7
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7f<<1);//GPIO C1-C7
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3f<<1);//GPIO C1-C6
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3f<<1);//GPIO C1-C6
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x1f<<1);//GPIO C1-C5
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x1f<<1);//GPIO C1-C5
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0xf<<1);//GPIO C1-C4
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0xf<<1);//GPIO C1-C4
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x7<<1);//GPIO C1-C3
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x7<<1);//GPIO C1-C3
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (0x3<<1);//GPIO C1-C2
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(0x3<<1);//GPIO C1-C2
        delay_us(10000);
        /***Pull high***/
        creg(0x2010) |= (1<<1);//GPIO C1
        delay_us(10000);
        /***Pull low***/
        creg(0x2010) &= ~(1<<1);//GPIO C1
        delay_us(10000);
        delay_us(30000);    
    }while(--n);
}

#define EIO_ID 0x44

static void eio_test(unsigned long n)
{
    unsigned char i;
    char buff;
    /***Clear pinmux***/    
    reg(0xc11080e0) &= ~(1<<31);reg(0xc11080c4) &= ~((1<<16)|(1<<22));reg(0xc11080d0) &= ~(1<<19);//GPIO D12
    
    reg(0xc110804c) &= ~(1<<10);
    reg(0xc1108048) &= ~(1<<10);
    delay_us(20000);
    reg(0xc110804c) |= (1<<10);
    delay_us(20000);
    
    hard_i2c_init(400);
    reg(0xc11080b8) &= ~(0x3f<<0);reg(0xc11080c8) &= ~(3<<28);reg(0xc11080b4) &= ~(1<<31);//AA10 & AB10     GPIOB0(SCL) & GPIOB1(SDA)     HW_I2C_SDA & HW_I2C_SCL
    reg(0xc11080b8) |= ((1<<2) | (1<<5));
    
    delay_us(10000);
    
    hard_i2c_write8(EIO_ID, 0x04, 0);
    hard_i2c_write8(EIO_ID, 0x05, 0);
    hard_i2c_write8(EIO_ID, 0x06, 0);
    hard_i2c_write8(EIO_ID, 0x0c, 0);
    hard_i2c_write8(EIO_ID, 0x0d, 0);
    hard_i2c_write8(EIO_ID, 0x0e, 0);
    delay_us(10000);
    
    do{
        buff = 0xff;
        
        hard_i2c_write8(EIO_ID, 0x04, 0xff);
        hard_i2c_write8(EIO_ID, 0x05, 0xff);
        hard_i2c_write8(EIO_ID, 0x06, 0xff);
        delay_us(10000);
        hard_i2c_write8(EIO_ID, 0x04, 0);
        hard_i2c_write8(EIO_ID, 0x05, 0);
        hard_i2c_write8(EIO_ID, 0x06, 0);
        delay_us(10000);
        
        for(i=0;i<7;i++){
            buff &= ~(1<<i);
            hard_i2c_write8(EIO_ID, 0x04, buff);
            hard_i2c_write8(EIO_ID, 0x05, buff);
            hard_i2c_write8(EIO_ID, 0x06, buff);
            delay_us(10000);
            hard_i2c_write8(EIO_ID, 0x04, 0);
            hard_i2c_write8(EIO_ID, 0x05, 0);
            hard_i2c_write8(EIO_ID, 0x06, 0);
            delay_us(10000);
        }
        
        delay_us(30000);
    }while(--n);
}

static void sth1_out32k(void)
{
    /***Clear pinmux***/    
    reg(0xc11080b0) &= ~(1<<16);reg(0xc11080b4) &= ~(1<<5);reg(0xc11080dc) &= ~(1<<1);reg(0xc11080e0) &= ~(1<<14);//GPIOA0
    
    /***Enable 32.768K***/
    reg(0xc11080d0) |= (1<<18);
    
    reg(0xc1108030) &= ~(1<<4);
}

static int do_gmtest(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd;
	
	char *endp;
	unsigned long loop;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	
	  if(argc > 2){
	      loop = simple_strtoul(argv[2], &endp, 10);
        if (*argv[2] == 0 || *endp != 0)
		        loop = 1000;
	  }
    
    if (strcmp(cmd, "0") == 0)
        sth1_out32k();
    else if (strcmp(cmd, "1") == 0)
        wifi_sdio_test(loop);
    else if (strcmp(cmd, "2") == 0)
        i2c_test(loop);
    else if (strcmp(cmd, "3") == 0)
        ir_test(loop);
    else if (strcmp(cmd, "4") == 0)
        itu601_test(loop);
    else if (strcmp(cmd, "5") == 0)
        eio_test(loop);
    else
        goto usage;

    return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	gmtest,	5,	1,	do_gmtest,
	"GPIO modules test",
	"0, STH1 32.768K output.\n"
	"1, wifi sdio test.\n"
	"2, I2C test.\n"
	"3, ir test.\n"
	"4, ITU601 test\n"
	"5, EIO test\n"
	"\n"
	""
);

#define TDATA32F 0xffffffff
#define TDATA32A 0xaaaaaaaa
#define TDATA325 0x55555555

static void ddr_write(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA32F;
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        *(p+i) = 0;
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        *(p+i) = TDATA32A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA325;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = 1<<j;
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = ~(1<<j);
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;                
                break;
            }
        }
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA325);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != 1<<j)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != ~(1<<j))
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), ~(1<<j));
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;
                break;
            }
        }
    }
}


#define DDR_TEST_START_ADDR 0x8d000000
#define DDR_TEST_SIZE 0x2000000

static int do_ddr_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	//const char *cmd;
	char *endp;
	unsigned long loop;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	//cmd = argv[1];
	
	loop = simple_strtoul(argv[1], &endp, 10);
	 if (*argv[1] == 0 || *endp != 0)
		        loop = 3;
	
	do{
        printf("Start writing at 0x%08x - 0x%08x...\n", DDR_TEST_START_ADDR, DDR_TEST_START_ADDR + DDR_TEST_SIZE);
	    ddr_write((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("End write.\n");
	    printf("Start reading...\n");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("End read.\n");
	    printf("Start reading...\n");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("End read.\n");
	    printf("Start reading...\n");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("End read.\n");
	  }while(--loop);
	  
	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	ddrtest,	5,	1,	do_ddr_test,
	"DDR test function",
	"ddrtest loop.\n"
);

static void (*s_powerdown)(void);

#define C_TIMER_MUX (*(volatile unsigned *)0xc1109940)
#define C_TIMER_E (*(volatile unsigned *)0xc1109954)

#define PREG(addr) *(volatile unsigned long *)(0xd0040000 + addr)
#define PARA_ADDR 0x49000030

static void powerdown(void)
{    
    reg(0xd0040004) = 3;
    C_TIMER_E = 0;
    while(C_TIMER_E < 300);
    reg(0xd0041380) |= 1;
    C_TIMER_E = 0;
    while(C_TIMER_E < 300);
    
    
    reg(0xc11041a0) |= (3<<16) | (0x4<<9);//(0x1f<<20) | (3<<16) | (0x1f<<9);
        
    C_TIMER_E = 0;
    while(C_TIMER_E < 16000);
        
    reg(0xc11041a0) = 0x110220;
    C_TIMER_E = 0;
    while(C_TIMER_E < 300);
        
    reg(0xd0041380) &= ~1;
    C_TIMER_E = 0;
    while(C_TIMER_E < 300);
    reg(0xd0040004) = 4;
}
    
static int do_powerdown(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{   
    s_powerdown = (void(*)(void))(0x49000100);
    memcpy((void *)0x49000100, (void *)powerdown, (unsigned)do_powerdown - (unsigned)powerdown);
    icache_invalid();
    dcache_flush();
    (*s_powerdown)();

    return 0;
    
usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	powerdown,	1,	1,	do_powerdown,
	"DDR Power down",
	"ddr power down test.\n"
);

#define RTC_COUNTER_ADDR            0
#define RTC_GPO_COUNTER_ADDR        1
#define RTC_SEC_ADJUST_ADDR         2
#define RTC_UNUSED_ADDR_0           3
#define RTC_REGMEM_ADDR_0           4
#define RTC_REGMEM_ADDR_1           5
#define RTC_REGMEM_ADDR_2           6
#define RTC_REGMEM_ADDR_3           7

static int do_rtc_test(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	const char *cmd, *cmd1;
	char *endp;
	unsigned long val;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	cmd = argv[1];
	
	  if (strcmp(cmd, "g") == 0){
        if (argc < 3)
		        goto usage;
		        
		    cmd1 = argv[2];
		    
		    if (strcmp(cmd1, "0") == 0){
		        ser_access_write(RTC_GPO_COUNTER_ADDR, 0x500000);
		    }
		    else if (strcmp(cmd1, "1") == 0){
		        ser_access_write(RTC_GPO_COUNTER_ADDR, 0x100000);
		    }
		    else{
		        goto usage;
        }
    }
    else if (strcmp(cmd, "sa") == 0){
        if (argc < 3)
		        goto usage;
		        
		    val = simple_strtoul(argv[2], &endp, 16);
	      if (*argv[2] == 0 || *endp != 0)
		        val = 0x4;
		    
		    rtc_ser_static_write_auto(val);
		}    
    else if (strcmp(cmd, "sm") == 0){
        if (argc < 3)
		        goto usage;
		        
		    val = simple_strtoul(argv[2], &endp, 16);
	      if (*argv[2] == 0 || *endp != 0)
		        val = 0x4;
		    
		    rtc_ser_static_write_manual(val);
		}
		else if (strcmp(cmd, "ia") == 0){
        rtc_ser_static_write_auto(0x4);
		}
		else if (strcmp(cmd, "rc") == 0){
		    while(1){
		        ser_access_read(RTC_COUNTER_ADDR);
		        delay_us1(1000000);
		    }
		}
		else if (strcmp(cmd, "wc") == 0){
		    val = simple_strtoul(argv[2], &endp, 16);
	      if (*argv[2] == 0 || *endp != 0)
		        val = 0x0;
		    ser_access_write(RTC_COUNTER_ADDR, val);
		}
		else if (strcmp(cmd, "co") == 0){
		    osc_clk_detect();
		}
    else{
        goto usage;
    }
	  
	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	rtc,	5,	1,	do_rtc_test,
	"RTC function",
	"RTC test.\n"
);

#define DEVICE_ID 0x68

typedef unsigned char ubyte;
//typedef unsigned short ushort;

static uchar DevID = DEVICE_ID;
static uchar iMode = 16;

extern ubyte hard_i2c_read8(ubyte SlaveAddr, ubyte RegAddr);
extern ushort hard_i2c_read16(ubyte SlaveAddr, ushort RegAddr);

static int do_i2c_func(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    const char *cmd;
	
	char *endp;
	unsigned long did, regAddr, data, mode, loop;
	ushort rd16;
	ubyte rd8;
	
	extern char console_buffer[];

	/* need at least two arguments */
	if (argc < 3)
		goto usage;

	cmd = argv[1];
    
    if (strcmp(cmd, "id") == 0) {
        did = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0){
            goto error;
        }
		
	    DevID = did;
	    printf("Device ID is modify to 0x%x.\n", DevID);
	    return 0;
    }
    else if (strcmp(cmd, "m") == 0) {
        mode = simple_strtoul(argv[2], &endp, 10);
        if (*argv[2] == 0 || *endp != 0){
            goto error;
        }
		
		iMode = mode;
		if(iMode == 16)
	       printf("I2C is modify to 16bit mode.\n");
	    else
	       printf("I2C is modify to 8bit mode.\n");
	    return 0;
    }
    else if (strcmp(cmd, "r") == 0) {
        regAddr = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0){
            goto error;
        }
		
		if(iMode == 16){
            rd16 = hard_i2c_read16(DevID, (ushort)regAddr);
            printf("Read data 0x%04x from 0x%04x[0x%02x]\n", rd16, (ushort)regAddr, DevID);
        }	    
	    else{
            rd8 = hard_i2c_read8(DevID, (ubyte)regAddr);
            printf("Read data 0x%02x from 0x%02x[0x%02x]\n", rd8, (ubyte)regAddr, DevID);
        }
	    
	    return 0;
    }
    else if (strcmp(cmd, "w") == 0) {
        if (argc == 3){
            printf ("Missing parameter!\n");
            goto usage;
        }
        
        regAddr = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0){
            goto error;
        }
        
        data = simple_strtoul(argv[3], &endp, 16);
        if (*argv[3] == 0 || *endp != 0){
            goto error;
        }
		
		if(iMode == 16){
            rd16 = hard_i2c_read16(DevID, (ushort)regAddr);
            printf("[0x%02x][0x%04x]0x%04x->", DevID, (ushort)regAddr, rd16);
            hard_i2c_write16(DevID, (ushort)regAddr, (ushort)data);
            rd16 = hard_i2c_read16(DevID, (ushort)regAddr);
            printf("0x%04x\n", rd16);
        }	    
	    else{
	        rd8 = hard_i2c_read8(DevID, (ubyte)regAddr);
	        printf("[0x%02x][0x%02x]0x%02x->", DevID, (ubyte)regAddr, rd8);
            hard_i2c_write8(DevID, (ubyte)regAddr, (ubyte)data);
            rd8 = hard_i2c_read8(DevID, (ubyte)regAddr);
            printf("0x%02x\n", rd8);
        }
	    
	    return 0;
    }
    else if (strcmp(cmd, "wl") == 0) {
        loop = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0){
            goto error;
        }

        while(loop--){
            printf("Reg:");
            readline(" ");
            regAddr = simple_strtoul(console_buffer, &endp, 16);
            //printf("0x%x\n", regAddr);
            printf("Data:");
            readline(" ");
            data = simple_strtoul(console_buffer, &endp, 16);
            //printf("0x%x\n", data);
            if(iMode == 16){
                hard_i2c_write16(DevID, (ushort)regAddr, (ushort)data);
            }
            else{
                rd8 = hard_i2c_read8(DevID, (ubyte)regAddr);
            }
        }

        return 0;
    }
    else{
        printf("\"%s\" is not a valid command.\n", cmd);
    }
    
error:   
     printf("Wrong format!");
usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	i2c,   5,  1,  do_i2c_func,
	"I2C interface function",
	"id [DEVICE ID]\n"
	"i2c m [MODE]. 16bit mode or 8bit mode.\n"
	"i2c r [REGISTER ADDRESS]. Read data from register address.\n"
	"i2c w [REGISTER ADDRESS] [DATA]. Write data to register address.\n"
);

static int do_ddr2pll(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *endp;
	unsigned long pll;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	pll = simple_strtoul(argv[1], &endp, 16);
	 if (*argv[1] == 0 || *endp != 0){
          printf ("Error: Wrong format parament!\n");
          return 1;
     }

     creg(0x207d) = pll;
     creg(0x207c) = 0xa2011;
     printf("Set DDR pll done [0x%08x]\n", creg(0x207d));
     reg(0xc1109900) = 0x400001;

	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	d2pll,	5,	1,	do_ddr2pll,
	"DDR PLL set function",
	"DDR PLL set: d2pll [PLL], e...g... 0x110221.\n"
);

static int do_sys2pll(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *endp;
	unsigned long pll;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	pll = simple_strtoul(argv[1], &endp, 16);
	 if (*argv[1] == 0 || *endp != 0){
          printf ("Error: Wrong format parament!\n");
          return 1;
     }

     creg(0x207d) = pll;
     creg(0x207c) = 0xb2011;
     printf("Set system pll done [0x%08x]\n", creg(0x207d));
     reg(0xc1109900) = 0x400001;

	  return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	s2pll,	5,	1,	do_sys2pll,
	"System PLL set function",
	"System PLL set: s2pll [PLL], e...g... 0x222.\n"
);

static int do_jtag2arc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
     creg(0x207c) = 0xda2011;
     reg(0xc1109900) = 0x400001;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	j2arc,	1,	1,	do_jtag2arc,
	"ARC JTAG enable function",
	"\n"
);