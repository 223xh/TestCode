#include <common.h>
#include <spi_flash.h>

#include <asm/io.h>
#include <asm/arch/io.h>
#include <asm/arch/register.h>

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
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA32A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), TDATA325);
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
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), 1<<j);
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
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len + i), ~(1<<j));
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
    char *endp;
    unsigned long loop;
    unsigned char lflag = 0;

	/* need at least two arguments */
    if (argc < 2)
        goto usage;

	//cmd = argv[1];

    if (strcmp(argv[1], "l") == 0){
        lflag = 1;
    }
    else{
        loop = simple_strtoul(argv[1], &endp, 10);
        if (*argv[1] == 0 || *endp != 0)
            loop = 3;
    }

    do{
        if(lflag)
            loop = 888;
        printf("\rStart writing at 0x%08x - 0x%08x...", DDR_TEST_START_ADDR, DDR_TEST_START_ADDR + DDR_TEST_SIZE);
        ddr_write((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read((void *)DDR_TEST_START_ADDR, DDR_TEST_SIZE);
	    printf("\rEnd 3rd read.                              \n");
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

static int do_ddr2pll(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *endp;
	unsigned long pll, zqcr;

	/* need at least two arguments */
	if (argc < 2)
		goto usage;

	pll = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0){
        printf ("Error: Wrong Fotmat parament!\n");
        return 1;
    }

    zqcr = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0){
        zqcr = 0;
    }

    WRITE_CBUS_REG(PREG_STICKY_REG0, 0x2012 | (zqcr << 16));
    WRITE_CBUS_REG(PREG_STICKY_REG1, pll);
    printf("Set pll done [0x%08x]\n", READ_CBUS_REG(PREG_STICKY_REG1));
    WRITE_CBUS_REG(WATCHDOG_TC, 0x400001);

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	d2pll,	5,	1,	do_ddr2pll,
	"DDR set PLL function",
	"DDR PLL set: d2pll [PLL], e...g... 0x110221.\n"
);

static int do_ssadc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    char *endp;
    unsigned long tmp;

    if(!argc)
        goto usage;

    if(argc > 1){
        tmp = simple_strtoul(argv[2], &endp, 10);
	    if (*argv[2] == 0 || *endp != 0){
            tmp = 0;
        }
    }

    if (strcmp(argv[1], "on") == 0){

        saradc_enable();

        printf("saradc is power on\n");
    }
    else if (strcmp(argv[1], "sw") == 0){

        saradc_temp_sw(tmp);
        printf("saradc temp mode is switch to %u\n", tmp);
    }
    else if (strcmp(argv[1], "off") == 0){
        saradc_disable();
        printf("saradc is power off\n");
    }
    else if (strcmp(argv[1], "get") == 0){
        printf("saradc[%u] value is %d\n", tmp, get_adc_sample(tmp));
    }
    else
        goto usage;

	return 0;

usage:
	cmd_usage(cmdtp);
	return 1;
}

U_BOOT_CMD(
	ssadc,	3,	1,	do_ssadc,
	"saradc test function",
	"....\n"
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
                printf("GPO is set from %x", ser_access_read(RTC_GPO_COUNTER_ADDR));
		        ser_access_write(RTC_GPO_COUNTER_ADDR, 0x500000);
		        printf("to %x\n", ser_access_read(RTC_GPO_COUNTER_ADDR));
		    }
		    else if (strcmp(cmd1, "1") == 0){
		        printf("GPO is set from %x", ser_access_read(RTC_GPO_COUNTER_ADDR));
		        ser_access_write(RTC_GPO_COUNTER_ADDR, 0x100000);
		        printf("to %x\n", ser_access_read(RTC_GPO_COUNTER_ADDR));
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
		        val = 0x180a;

		    rtc_ser_static_write_auto(val);
		}
    else if (strcmp(cmd, "sm") == 0){
        if (argc < 3)
		        goto usage;

		    val = simple_strtoul(argv[2], &endp, 16);
	      if (*argv[2] == 0 || *endp != 0)
		        val = 0x180a;

		    rtc_ser_static_write_manual(val);
		}
		else if (strcmp(cmd, "ia") == 0){
        rtc_ser_static_write_auto(0x180a);
		}
		else if (strcmp(cmd, "rc") == 0){
		    while(1){
		        ser_access_read(RTC_COUNTER_ADDR);
		        //delay_us1(1000000);
		        __udelay(1000000);
		    }
		}
		else if (strcmp(cmd, "wc") == 0){
		    val = simple_strtoul(argv[2], &endp, 16);
	      if (*argv[2] == 0 || *endp != 0)
		        val = 0x0;
		    ser_access_write(RTC_COUNTER_ADDR, val);
		}/*
		else if (strcmp(cmd, "co") == 0){
		    check_osc_clk();
		}*/
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
