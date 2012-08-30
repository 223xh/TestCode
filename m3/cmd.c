#include <common.h>

#include <asm/io.h>
#include <asm/arch/io.h>
#include <asm/arch/register.h>

#define creg(a) (*(volatile unsigned*)(0xc1100000 + ((a)<<2)))
#define reg(a) (*(volatile unsigned*)(a))

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
    unsigned long loop = 1;
    unsigned char lflag = 0;
    unsigned start_addr = DDR_TEST_START_ADDR;

    if(!argc)
        goto DDR_TEST_START;

    if (strcmp(argv[1], "l") == 0){
        lflag = 1;
    }
    else if (strcmp(argv[1], "h") == 0){
        goto usage;
    }
    else{
        loop = simple_strtoul(argv[1], &endp, 10);
        if (*argv[1] == 0 || *endp != 0)
            loop = 1;
    }

    if(argc > 2){
        start_addr = simple_strtoul(argv[2], &endp, 16);
        if (*argv[2] == 0 || *endp != 0)
            start_addr = DDR_TEST_START_ADDR;
    }

DDR_TEST_START:

    do{
        if(lflag)
            loop = 888;
        printf("\rStart writing at 0x%08x - 0x%08x...", start_addr, start_addr + DDR_TEST_SIZE);
        ddr_write((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd write.                                 ");
	    printf("\rStart 1st reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 1st read.                              ");
	    printf("\rStart 2nd reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
	    printf("\rEnd 2nd read.                              ");
	    printf("\rStart 3rd reading...                       ");
	    ddr_read((void *)start_addr, DDR_TEST_SIZE);
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
	"ddrtest [LOOP] [ADDR].Default address is 0x8d000000\n"
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
        printf ("Error: Wrong format parament!\n");
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
	"DDR PLL set: d2pll PLL [ZQCR], e...g... 0x1022c.\n"
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

#define DEVICE_ID 0xb6

typedef unsigned char ubyte;
//typedef unsigned short ushort;

static uchar DevID = DEVICE_ID;
static uchar iMode = 8;

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
	i2c,   6,  1,  do_i2c_func,
	"I2C interface function",
	"id [DEVICE ID]\n"
	"i2c m [MODE]. 16bit mode or 8bit mode.\n"
	"i2c r [REGISTER ADDRESS]. Read data from register address.\n"
	"i2c w [REGISTER ADDRESS] [DATA]. Write data to register address.\n"
);
