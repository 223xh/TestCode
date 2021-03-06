
#include <common.h>
#include <asm/mach-types.h>
#include <asm/arch/memory.h>
#include <asm/arch/am_reg_addr.h>
#include <asm/arch/aml_card_sd.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/am_eth_reg.h>
#include <asm/arch/clk_set.h>
#include <share_kernel.h>
#include <asm/arch/gpio.h>
#include <asm/arch/nand.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <asm/arch/osd.h>
#include <asm/arch/osd_hw.h>
#include <asm/arch-m1/tv_out.h>
#include <asm/arch-m3/usb.h>


#include <asm/arch/i2c.h>
#include <aml_i2c.h>

/**
 * Routine: board_init
 * Description: Early hardware init.
 */
//just compile pass 
static struct aml_nand_platform aml_nand_mid_platform[] = {
#ifndef CONFIG_SPI_BOOT
   {
        .name = NAND_BOOT_NAME,
        .chip_enable_pad = AML_NAND_CE0,
        .ready_busy_pad = AML_NAND_CE0,
        .platform_nand_data = {
            .chip =  {
                .nr_chips = 1,
				.options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    },
#endif
    {
		.name = NAND_MULTI_NAME,
		.chip_enable_pad = (AML_NAND_CE0 | (AML_NAND_CE1 << 4) | (AML_NAND_CE2 << 8) | (AML_NAND_CE3 << 12)),
		.ready_busy_pad = (AML_NAND_CE0 | (AML_NAND_CE0 << 4) | (AML_NAND_CE1 << 8) | (AML_NAND_CE1 << 12)),
        .platform_nand_data = {
            .chip =  {
				.nr_chips = 4,
				.options = (NAND_TIMING_MODE5 | NAND_ECC_BCH60_1K_MODE | NAND_TWO_PLANE_MODE),
            },
        },
        .T_REA = 20,
        .T_RHOH = 15,
    }
};

struct aml_nand_device aml_nand_mid_device = {
    .aml_nand_platform = aml_nand_mid_platform,
    .dev_num = ARRAY_SIZE(aml_nand_mid_platform),
};

#ifdef CONFIG_USB_DWC_OTG_HCD
static void set_usb_b_vbus_power(char is_power_on)
{ 
	#define USB_B_POW_GPIO         GPIOD_bank_bit0_9(9)
	#define USB_B_POW_GPIO_BIT     GPIOD_bit_bit0_9(9)
	#define USB_B_POW_GPIO_BIT_ON   1
	#define USB_B_POW_GPIO_BIT_OFF  0

  printk(KERN_INFO "set usb b port power on (board gpio %d)!\n",USB_B_POW_GPIO_BIT);
  set_gpio_mode(USB_B_POW_GPIO, USB_B_POW_GPIO_BIT, GPIO_OUTPUT_MODE);
  set_gpio_val(USB_B_POW_GPIO, USB_B_POW_GPIO_BIT, USB_B_POW_GPIO_BIT_ON);
}

struct amlogic_usb_config usb_config_m3={
	USB_PHY_CLOCK_SEL_XTAL_DIV2,
	0, // no use if clock sel == xtal or xtal/2
	CONFIG_M3_USBPORT_BASE,
	USB_ID_MODE_SW_HOST,
	set_usb_b_vbus_power, //set_vbus_power
};
#endif

#ifdef CONFIG_PMU_ACT8942
#include <act8942.h>  

#define msleep(a) udelay(a * 1000)

static void power_off(void)
{
    //Power hold down
    set_gpio_val(GPIOAO_bank_bit0_11(6), GPIOAO_bit_bit0_11(6), 0);
    set_gpio_mode(GPIOAO_bank_bit0_11(6), GPIOAO_bit_bit0_11(6), GPIO_OUTPUT_MODE);
}



/*
 *	DC_DET(GPIOA_20)	enable internal pullup
 *		High:		Disconnect
 *		Low:		Connect
 */
static inline int is_ac_online(void)
{
	int val;
	
	SET_CBUS_REG_MASK(PAD_PULL_UP_REG0, (1<<20));	//enable internal pullup
	set_gpio_mode(GPIOA_bank_bit0_27(20), GPIOA_bit_bit0_27(20), GPIO_INPUT_MODE);
	val = get_gpio_val(GPIOA_bank_bit0_27(20), GPIOA_bit_bit0_27(20));
	
	printf("%s: get from gpio is %d.\n", __FUNCTION__, val);
	
	return !val;
}

//temporary
static inline int is_usb_online(void)
{
	u8 val;

	return 0;
}


/*
 *	nSTAT OUTPUT(GPIOA_21)	enable internal pullup
 *		High:		Full
 *		Low:		Charging
 */
static inline int get_charge_status(void)
{
	int val;
	
	SET_CBUS_REG_MASK(PAD_PULL_UP_REG0, (1<<21));	//enable internal pullup
	set_gpio_mode(GPIOA_bank_bit0_27(21), GPIOA_bit_bit0_27(21), GPIO_INPUT_MODE);
	val = get_gpio_val(GPIOA_bank_bit0_27(21), GPIOA_bit_bit0_27(21));

	printf("%s: get from gpio is %d.\n", __FUNCTION__, val);
	
	return val;
}

/*
 *	When BAT_SEL(GPIOA_22) is High Vbat=Vadc*2
 */
static inline int measure_voltage(void)
{
	int val;
	msleep(2);
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 1);
	val = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: get from adc is %duV.\n", __FUNCTION__, val);
	return val;
}

/*
 *	Get Vhigh when BAT_SEL(GPIOA_22) is High.
 *	Get Vlow when BAT_SEL(GPIOA_22) is Low.
 *	I = Vdiff / 0.02R
 *	Vdiff = Vhigh - Vlow
 */
static inline int measure_current(void)
{
	int val, Vh, Vl, Vdiff;
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 1);
	msleep(2);
	Vl = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: Vh is %duV.\n", __FUNCTION__, Vh);
	set_gpio_mode(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOA_bank_bit0_27(22), GPIOA_bit_bit0_27(22), 0);
	msleep(2);
	Vh = get_adc_sample(5) * (2 * 2500000 / 1023);
	printf("%s: Vl is %duV.\n", __FUNCTION__, Vl);
	Vdiff = Vh - Vl;
	val = Vdiff * 50;
	printf("%s: get from adc is %duA.\n", __FUNCTION__, val);
	return val;
}

static inline int measure_capacity(void)
{
	int val, tmp;
	tmp = measure_voltage();
	if((tmp>4200000) || (get_charge_status() == 0x1))
	{
		printf("%s: get from PMU and adc is 100.\n", __FUNCTION__);
		return 100;
	}
	
	val = (tmp - 3600000) / (600000 / 100);
	printf("%s: get from adc is %d.\n", __FUNCTION__, val);
	return val;
}

//temporary
static int set_bat_off(void)
{
	return 0;
}

static struct act8942_operations act8942_pdata = {
	.is_ac_online = is_ac_online,
	.is_usb_online = is_usb_online,
	.set_bat_off = set_bat_off,
	.get_charge_status = NULL,
	.measure_voltage = measure_voltage,
	.measure_current = measure_current,
	.measure_capacity_charging = measure_capacity,
	.measure_capacity_battery = measure_capacity,
	.update_period = 2000,	//2S
};

#endif

int board_init(void)
{
    /*
    @todo implement this function
    */
	DECLARE_GLOBAL_DATA_PTR;
	gd->bd->bi_arch_number=2958;
	gd->bd->bi_boot_params=BOOT_PARAMS_OFFSET;

#ifdef CONFIG_CMD_KGDB

	kgdb_init();
#endif

#ifdef CONFIG_USB_DWC_OTG_HCD
	board_usb_init(&usb_config_m3);
#endif

	return 0;
}

/*
 * Routine: misc_init_r
 * Description: Init ethernet (done here so udelay works)
 */
int misc_init_r(void)
{
/*
    @todo implement this function
    */

	return 0;
}

/*
 * Routine: set_muxconf_regs
 * Description: Setting up the configuration Mux registers specific to the
 *		hardware. Many pins need to be moved from protect to primary
 *		mode.
 */
void set_muxconf_regs(void)
{
    /*
    @todo implement this function
    */
}

#if defined(CONFIG_CMD_NET)
/*
 * Routine: setup_net_chip
 * Description: Setting up the configuration GPMC registers specific to the
 *		Ethernet hardware.
 */
static void setup_net_chip(void)
{
    /*
    @todo implement this function
    */
	
}

extern int aml_eth_init(bd_t *bis);
int board_eth_init(bd_t *bis)
{
	unsigned v;
	
	CLEAR_CBUS_REG_MASK(PERIPHS_PIN_MUX_6,(3<<17));//reg6[17/18]=0
#ifdef CONFIG_NET_CLK_EXTERNAL
	//rmii 50 in
	//set clock
    eth_clk_set_invert(7,50*CLK_1M,50*CLK_1M);
  eth_set_pinmux(ETH_BANK0_GPIOY1_Y9, ETH_CLK_IN_GPIOY0_REG6_18, 0);
#else
	/* Use Misc PLL for the source of ethernet */
	eth_clk_set(ETH_CLKSRC_MISC_CLK, get_misc_pll_clk(), (50 * CLK_1M));
	/* Use Internal clock output from GPIOY0*/
	eth_set_pinmux(ETH_BANK0_GPIOY1_Y9, ETH_CLK_OUT_GPIOY0_REG6_17, 0);
#endif //CONFIG_NET_CLK_EXTERNAL
	
	/*disalbe*/
	//reset:LCD_G5
	writel(readl(ETH_PLL_CNTL) & ~(1 << 0), ETH_PLL_CNTL); // Disable the Ethernet clocks     
	// ---------------------------------------------
	// Test 50Mhz Input Divide by 2
	// ---------------------------------------------
	// Select divide by 2
	writel(readl(ETH_PLL_CNTL) | (0 << 3), ETH_PLL_CNTL); // desc endianess "same order"   
	writel(readl(ETH_PLL_CNTL) | (0 << 2), ETH_PLL_CNTL); // data endianess "little"    
	writel(readl(ETH_PLL_CNTL) | (1 << 1), ETH_PLL_CNTL); // divide by 2 for 100M     
	writel(readl(ETH_PLL_CNTL) | (1 << 0), ETH_PLL_CNTL);  // enable Ethernet clocks   
	udelay(100);

	/* reset phy with GPIOD_7*/
	set_gpio_mode(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), GPIO_OUTPUT_MODE);
	set_gpio_val(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), 0);
	udelay(2000);    
	set_gpio_val(GPIOD_bank_bit0_9(7), GPIOD_bit_bit0_9(7), 1);
	udelay(2000);	//waiting reset end;
	
	/* reset phy with GPIOAO_6*/
/*	v = ~((1<<6)|(1<22));
	writel(v,0xC81000024);
	udelay(2000);   
	v |= (1<22);
	writel(v,0xC81000024);
	udelay(2000);	//waiting reset end;
	*/
    aml_eth_init(bis);
    return 0;
}
#endif /* (CONFIG_CMD_NET) */

u32 get_board_rev(void)
{
    /*
    @todo implement this function
    */
	return 0x20;
}
static int  sdio_init(unsigned port)
{
	//setbits_le32(P_PREG_CGPIO_EN_N,1<<5);
    //todo add card detect 	
	setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6

    return cpu_sdio_init(port);
}
static int  sdio_detect(unsigned port)
{
	setbits_le32(P_PREG_PAD_GPIO5_EN_N,1<<29);//CARD_6
	return readl(P_PREG_PAD_GPIO5_I)&(1<<29)?1:0;
}
static void sdio_pwr_prepare(unsigned port)
{
    /// @todo NOT FINISH
	///do nothing here
}
static void sdio_pwr_on(unsigned port)
{
	clrbits_le32(P_PREG_PAD_GPIO5_O,(1<<31));
	clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));
    /// @todo NOT FINISH
}
static void sdio_pwr_off(unsigned port)
{
//	setbits_le32(P_PREG_CGPIO_O,(1<<5));
//	clrbits_le32(P_PREG_CGPIO_EN_N,(1<<5));//test_n
	setbits_le32(P_PREG_PAD_GPIO5_O,(1<<31));
	clrbits_le32(P_PREG_PAD_GPIO5_EN_N,(1<<31));//GPIOD13
	/// @todo NOT FINISH
}
static void board_mmc_register(unsigned port)
{
    AML_CARD_SD_INFO_t *aml_priv=cpu_sdio_get(port);
    if(aml_priv==NULL)
        return;
    struct mmc *mmc = (struct mmc *)malloc(sizeof(struct mmc));
    strncpy(mmc->name,aml_priv->name,31);
    mmc->priv = aml_priv;
	aml_priv->removed_flag = 1;
	aml_priv->inited_flag = 0;
	aml_priv->sdio_init=sdio_init;
	aml_priv->sdio_detect=sdio_detect;
	aml_priv->sdio_pwr_off=sdio_pwr_off;
	aml_priv->sdio_pwr_on=sdio_pwr_on;
	aml_priv->sdio_pwr_prepare=sdio_pwr_prepare;
	mmc->send_cmd = aml_sd_send_cmd;
	mmc->set_ios = aml_sd_cfg_swth;
	mmc->init = aml_sd_init;
	mmc->rca = 1;
	mmc->voltages = MMC_VDD_33_34;
	mmc->host_caps = MMC_MODE_4BIT | MMC_MODE_HS;
	//mmc->host_caps = MMC_MODE_4BIT;
	mmc->bus_width = 1;
	mmc->clock = 300000;
	mmc->f_min = 200000;
	mmc->f_max = 50000000;
	mmc_register(mmc);
}
int board_mmc_init(bd_t	*bis)
{
//	board_mmc_register(SDIO_PORT_A);
	board_mmc_register(SDIO_PORT_B);
//	board_mmc_register(SDIO_PORT_C);
//	board_mmc_register(SDIO_PORT_B1);
	return 0;
}

struct aml_i2c_platform aml_i2c_plat = {
    .wait_count     = 1000000,
    .wait_ack_interval  = 5,
    .wait_read_interval = 5,
    .wait_xfer_interval = 5,
    .master_no      = MASTER_AO,
    .use_pio            = 0,
    .master_i2c_speed   = AML_I2C_SPPED_400K,

    .master_ao_pinmux = {
        .scl_reg    = P_AO_RTI_PIN_MUX_REG,
        .scl_bit    = (1<<6),
        .sda_reg    = P_AO_RTI_PIN_MUX_REG,
        .sda_bit    = (1<<5),
    }
};

static int is_ac_connected(void)
{
	return pmu_is_ac_online();
}

extern int osd_width;
extern int osd_height;

#include <bmp_layout.h>
//Not full test down scale function
extern bmp_image_t *gunzip_bmp(unsigned long addr, unsigned long *lenp);
void bmp_scale(ulong src_addr, ulong dst_addr, __u32 new_width, __u32 new_height)
{
	printf("Begin scale logo...\n");
	bmp_image_t *bmp = (bmp_image_t *)src_addr;
	bmp_image_t *bmp_dst = (bmp_image_t *)dst_addr;	
	unsigned long len;

	char *pBuf = (char*)bmp+bmp->header.data_offset;

	if (!((bmp->header.signature[0]=='B') &&
		      (bmp->header.signature[1]=='M')))
			bmp = gunzip_bmp(src_addr, &len);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return;
	}	      

	memcpy(bmp_dst, bmp, sizeof(bmp_image_t));
	bmp_dst->header.width=new_width;
	bmp_dst->header.height=new_height;
	char *pBuf_dst = (char*)bmp_dst+bmp_dst->header.data_offset;

#if 1//Fast scale
	int   nWidth   ,   nHeight	 ,	 nNewWidth	 ,	 nNewHeight   ,   nNewWidthBit	 ,	 nWidthBit; 
	float m_xscale,m_yscale;
	int i,j,x,y,oldoffset;	
	char *pNewTmp = NULL; 
	
	m_xscale = (float)bmp_dst->header.width/(float)bmp->header.width;
	m_yscale = (float)bmp_dst->header.height/(float)bmp->header.height;
	nWidth = bmp->header.width;
	nHeight = bmp->header.height;
	nNewHeight = bmp_dst->header.width;
	nNewWidth =	bmp_dst->header.width;
	nNewWidthBit = ( 4 - nNewWidth * 3 % 4 )%4 + nNewWidth * 3;

	for( i=0; i<nNewHeight; i++ )
	{ 
		pNewTmp = pBuf_dst + nNewWidthBit * i; 
		for( j=0; j<nNewWidth * 3; j += 3 ) 
		{ 
			x = (int) (j/m_xscale); 
			y = (int) (i/m_yscale); 
			oldoffset = (y*nWidth*3 + x) - (y*nWidth*3 + x)%3; //correct positon in 3 byte mode
			memcpy(pNewTmp+j, pBuf + oldoffset, 3);
		} 
	} 
#endif


#if 0//High quality
	int sw = bmp->header.width - 1, sh = bmp->header.height- 1, dw = bmp_dst->header.width - 1, dh = bmp_dst->header.height- 1;
    int B, N, x, y;
    int nPixelSize = bmp->header.bit_count/8;
    char * pLinePrev, *pLineNext;
	char *pSrc,*pDest;    
    char * pA, *pB, *pC, *pD;
    unsigned int i,j,k;
    for ( i = 0; i <= dh; ++i )
    {
        pDest = ( char * )(pBuf_dst+bmp_dst->header.width*i*nPixelSize);
        y = i * sh / dh;
        N = dh - i * sh % dh;
        pLinePrev = ( char * )(pBuf+bmp->header.width*y*nPixelSize);
        y++;
        pLineNext = ( N == dh ) ? pLinePrev : ( char * )(pBuf+bmp->header.width*y*nPixelSize);
        for ( j = 0; j <= dw; ++j )
        {
            x = j * sw / dw * nPixelSize;
            B = dw - j * sw % dw;
            pA = pLinePrev + x;
            pB = pA + nPixelSize;
            pC = pLineNext + x;
            pD = pC + nPixelSize;
            if ( B == dw )
            {
                pB = pA;
                pD = pC;
            }
            for ( k = 0; k < nPixelSize; ++k )
                *pDest++ = ( char )( int )(
                    ( B * N * ( *pA++ - *pB - *pC + *pD ) + dw * N * *pB++
                    + dh * B * *pC++ + ( dw * dh - dh * B - dw * N ) * *pD++
                    + dw * dh / 2 ) / (double)( dw * dh )
                );
        }
    }

#endif


	printf("End scale logo.\n");

}

int logo_display(void)
{
    int ret = 0;
	char buf[32];
	
	printf("logo_display: \n");

	run_command ("osd enable", 0);
	sprintf(buf, "osd open 24 %d %d 1", osd_width, osd_height);
    printf("buf=%s.\n", buf);
    run_command (buf, 0);
    run_command ("osd fill 0", 0);

    run_command ("tv open ${outputmode}", 0);
//	run_command ("nand read ${loadaddr} ${aml_logo_start} ${aml_logo_size}", 0);
//	run_command ("nand read ${aml_logo_name} ${loadaddr} 0 ${aml_logo_size}", 0);
//    ret = run_command ("bmp display ${loadaddr}", 0);
	run_command ("nand read ${aml_logo_name} 82000000 0 ${aml_logo_size}", 0);

   bmp_scale(0x82000000, 0x84100000, osd_width, osd_height);
	 
    ret = run_command ("bmp display 84100000", 0);
//    run_command ("lcd bl on", 0);

#ifdef M3_DSP_VSYNC_INTERRUPT
		ret = run_command ("tvdsp check;tvdsp stop", 0);
#endif

    return ret;

}

inline void display_messge(char *msg)
{
#ifdef ENABLE_FONT_RESOURCE    
    run_command ("lcd clear", 0);
    //AsciiPrintf(msg, 250, 200, 0x80ff80);
    AsciiPrintf(msg, 0, 0, 0x00ff00);
    run_command ("lcd bl on", 0);
#else
	printf("%s\n",msg);
#endif    
}
//end of Dirty


//Power Function
inline void power_up(void)
{
	clrbits_le32(P_AO_GPIO_O_EN_N, ((1<<2)|(1<<6)));
	setbits_le32(P_AO_GPIO_O_EN_N,((1<<18)|(1<<22)));
}

inline void power_down()
{
	clrbits_le32(P_AO_GPIO_O_EN_N, ((1<<2)|(1<<6)));
	setbits_le32(P_AO_GPIO_O_EN_N,((1<<18)|(1<<22)));
}
//end of Power Function

//powerkey function
static  int powerkey_hold_count = -1; 

static inline int powerkey_init(void)
{
	clrbits_le32(P_RTC_ADDR0, (1<<11));
	clrbits_le32(P_RTC_ADDR1, (1<<3));
    return 0;
}

int powerkey_scan(void)
{
	return (((readl(P_RTC_ADDR1) >> 2) & 1) ? 0 : 1);
}

int powerkey_hold(unsigned long hold_time)
{
    unsigned long polling_time = 100, tmp;

    if(powerkey_hold_count == -1)
    {
        powerkey_init();
        
    }

    if(!hold_time)
    {
        tmp = powerkey_scan();
        printf("powerkey: %d\n", tmp);
        if((!tmp) || (powerkey_hold_count < 0))
        {
            powerkey_hold_count = -2;
            return  0;
        }
        else
        {
            return  ++powerkey_hold_count;
        }
    }

    while(hold_time > 0)
    {
        mdelay(polling_time);
        tmp = powerkey_scan();
        //printf("powerkey: %d\n", tmp);
        if(!tmp)  break;
        if(hold_time <= polling_time)
        {
            hold_time = 0;
        }
        else
        {
            hold_time -= polling_time;
        }
    }
    if(hold_time > 0)
    {
        return  0;
    }
    return  1;
}
//end of powerkey function

//Upgrade Function
#define NAND_CLEAN

int upgrade_bootloader(void)
{
    int	i = 0, j = 0, retry = 2;
    char	str[128];
    unsigned long   size, size1;
    char    *filepath;
    
    printf("u-boot upgrading...\n");
    if(run_command ("mmcinfo", 0))
    {
        printf("##	ERROR: SD card not find!!!\n");
    }
    else
    {
        printf("Find SD card!!!\n");

        for(i = 0; i < SCAN_DEVICE_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d ${bootloader_path}", (i + 1));
            printf("command:    %s\n", str);
            if(!run_command (str, 0))
            {
				sprintf(str, "fatload mmc 0:%d ${loadaddr} ${bootloader_path}", (i + 1));
				printf("command:    %s\n", str);
				run_command (str, 0);
				run_command ("sf probe 2", 0);
				run_command ("sf erase 0 60000", 0);
				run_command ("sf write 82000000 0 60000", 0);

				return 1;


#if 0
				size = simple_strtoul (getenv ("bootloader_filesize"), NULL, 16);
                size1 = simple_strtoul (getenv ("filesize"), NULL, 16);
                //if(size != size1)
                {
                    printf("bootloader_filesize:%d   !=  filesize:%d\n", size, size1);
                    while(retry-- > 0)
                    {
                        sprintf(str, "fatload mmc 0:%d ${loadaddr} ${bootloader_path}", (i + 1));
                        printf("command:    %s\n", str);
                        run_command (str, 0);

                        run_command ("nand rom_protect off", 0);
#ifdef  NAND_CLEAN
						run_command ("nand scrub", 0);	
#endif				
                        run_command ("nand rom_write ${loadaddr} ${bootloader_start} ${bootloader_size}", 0);
                        run_command ("nand rom_protect on", 0);

                        size = simple_strtoul (getenv ("filesize"), NULL, 16);

                        sprintf(str, "set bootloader_filesize 0x%x", size);
                        run_command (str, 0);

                        printf("bootloader upgrade successful!\n");
                        run_command ("save", 0);
                        return 1;
                    }
                }
#endif
            }
        }
    }
    return 0;
}



void into_recovery(void)
{
    int	i = 0, j = 0, ret = 0;
    char	str[128];
    
    printf("Recovery Start...\n");
    //run_command ("nand read 84100000 ${logo_start} ${logo_size}", 0);
    if(!run_command ("mmcinfo", 0))
    {
        for(i = 0; i < SCAN_DEVICE_PARTITION; i++)
        {
            sprintf(str, "fatexist mmc 0:%d ${recovery_path}", (i + 1));
            if(!run_command (str, 0))
            {
                printf("recovery in SD Card!!!\n");
                sprintf(str, "fatload mmc 0:%d ${loadaddr} ${recovery_path}", (i + 1));
                run_command (str, 0);
                run_command ("bootm", 0);
            }
        }
    }

    printf("recovery in nand!!!\n");
    run_command ("nand read ${recovery_name} ${loadaddr} 0 ${recovery_size}", 0);

    run_command ("bootm", 0);
}
//end of Upgrade Function

int scan_mode = SCAN_MODE_PROGRESSIVE;
int osd_width=1280;
int osd_height=720;


static int get_tv_mode(char *mode_name)
{
	if((strcmp(mode_name, "1080P")==0) || (strcmp(mode_name, "1080p")==0))
	{
       scan_mode = 1; // SCAN_MODE_PROGRESSIVE;
		return TVOUT_1080P;
	}
	if((strcmp(mode_name, "1080I")==0) || (strcmp(mode_name, "1080i")==0))
	{
       scan_mode = 0; // SCAN_MODE_INTERLACE;
		return TVOUT_1080I;
	}
	if((strcmp(mode_name, "720P")==0) || (strcmp(mode_name, "720p")==0))
	{
       scan_mode = 1; // SCAN_MODE_PROGRESSIVE;
		return TVOUT_720P;
	}
	if((strcmp(mode_name, "576P")==0) || (strcmp(mode_name, "576p")==0))
	{
       scan_mode = 1; // SCAN_MODE_PROGRESSIVE;
		return TVOUT_576P;
	}
	if((strcmp(mode_name, "480P")==0) || (strcmp(mode_name, "480p")==0))
	{
       scan_mode = 1; // SCAN_MODE_PROGRESSIVE;
		return TVOUT_480P;
	}
	if((strcmp(mode_name, "576I")==0) || (strcmp(mode_name, "576i")==0))
	{
       scan_mode = 0; // SCAN_MODE_INTERLACE;
		return TVOUT_576I;
	}
	if((strcmp(mode_name, "480I")==0) || (strcmp(mode_name, "480i")==0))
	{
       scan_mode = 0; // SCAN_MODE_INTERLACE;
		return TVOUT_480I;
	}
	if((strcmp(mode_name, "vga")==0) || (strcmp(mode_name, "VGA")==0))
	{
       scan_mode = 1;
		return TVMODE_VGA;
	}
	if((strcmp(mode_name, "svga")==0) || (strcmp(mode_name, "SVGA")==0))
	{
       scan_mode = 1;
		return TVMODE_SVGA;
	}
	if((strcmp(mode_name, "xga")==0) || (strcmp(mode_name, "XGA")==0))
	{
       scan_mode = 1;
		return TVMODE_XGA;
	}
	if((strcmp(mode_name, "sxga")==0) || (strcmp(mode_name, "SXGA")==0))
	{
       scan_mode = 1;
		return TVMODE_SXGA;
	}
	return TVOUT_MAX;
}

// for mbox, output mode could be changed by user, so osd size is determinate,
// we need to get osd size from env.
void get_osd_size()
{
    char buf[32];
	char tvmode[16];
	char * str;
	
//	printf("get_osd_size: \n");
	str = getenv("outputmode");
	sprintf(tvmode, "%s", str);

	tv_out_get_info(get_tv_mode(tvmode), &osd_width, &osd_height);

/*
    if(strstr(tvmode, "i") || strstr(tvmode, "I"))
        scan_mode = 0; // SCAN_MODE_INTERLACE;
	else
       scan_mode = 1; // SCAN_MODE_PROGRESSIVE;
	
	sprintf(buf, "%soutputwidth", tvmode);
	str = getenv(buf);
    osd_width = 	str ? (int)simple_strtol(str, NULL, 10) : 1280;
	
	sprintf(buf, "%soutputheight", tvmode);
	str = getenv(buf);
    osd_height = 	str ? (int)simple_strtol(str, NULL, 10) : 720;
*/
}



#ifdef CONFIG_SWITCH_BOOT_MODE

inline int get_key()
{
    int adc_val = get_adc_sample(4);
    printf("get_adc_sample(4): 0x%x\n", adc_val);
    return(((adc_val >= 0x0) && (adc_val < 0x50)) ? 1 : 0);
}

int switch_boot_mode(void)
{
    unsigned long hold_time = 50000, polling_time = 10000, tmp;
    unsigned long upgrade_step;
    
    clrbits_le32(P_PREG_PAD_GPIO2_EN_N, (1<<25));     // VCC5V_EN GPIOD_9
    setbits_le32(P_PREG_PAD_GPIO2_O, (1<<25));

	get_osd_size();
	
#ifdef CONFIG_PMU_ACT8942
	act8942_init(&act8942_pdata);
	//act8942_dump();
#endif

	upgrade_step = simple_strtoul (getenv ("upgrade_step"), NULL, 16);
	printf("upgrade_step = %d\n", upgrade_step);
	
	saradc_enable();
	
#ifdef ENABLE_FONT_RESOURCE
	RegisterFont(DEFAULT_FONT);
#endif
	aml_autoscript();
	powerkey_hold(0);
	if(upgrade_step == 2)
	{
		switch(reboot_mode)
		{
			case AMLOGIC_NORMAL_BOOT:
			{
				printf("AMLOGIC_NORMAL_BOOT...\n");
				power_up();
				logo_display();
				return	1;
			}
			case AMLOGIC_FACTORY_RESET_REBOOT:
			{
				printf("AMLOGIC_FACTORY_RESET_REBOOT...\n");
				power_up();
				logo_display();
				run_command ("nand read ${recovery_name} ${loadaddr} 0 ${recovery_size}", 0);
				run_command ("bootm", 0);
				break;
			}
			case AMLOGIC_UPDATE_REBOOT:
			{
				printf("AMLOGIC_UPDATE_REBOOT...\n");
				power_up();
				logo_display();
				run_command ("set upgrade_step 0", 0);
				run_command ("save", 0);
				upgrade_step = 0;
				break;
			}
			default:
			{
				printf("AMLOGIC_CHARGING_REBOOT...\n");
				if(is_ac_connected)
				{
					power_up();
#ifdef CONFIG_BATTERY_CHARGING
					battery_charging();
#endif
					logo_display();
				}
				else
				{
					powerkey_hold(0);
#ifdef CONFIG_BATTERY_CHARGING
					if(get_powerkey_hold_count())
					{
						  logo_display();
						if(get_battery_percentage() < 10)
						{
							power_low_display();
							sdelay(2);
							power_down();
							printf("Low Power!!!\nPower Down!\n");
							hang();
						}
						logo_display();
						power_up();
						printf("Power Up!\n");
					}
#else
					if(powerkey_hold(1000))
					{
						logo_display();
						power_up();
						printf("Power Up!\n");
					}
#endif
					else
					{
						power_down();
						printf("Power Down!\n");
						hang();
					}
				}
				break;
			}
		}
	}
	else
	{
		power_up();
		printf("Upgrade step %d...\n", upgrade_step);
	}

	if(upgrade_step == 0)
	{
		display_messge("upgrade step 1! Don't Power Off!");
		if(upgrade_bootloader())
		{
			run_command ("set upgrade_step 1", 0);
			run_command ("save", 0);
			run_command ("reset", 0);
			hang();
		}
		else
		{
			printf("### ERROR:	u-boot write failed!!!\n");
			return	-1;
		}
	}
	else if(upgrade_step == 1)
	{
		display_messge("upgrade step 2! Don't Power Off!");
		run_command ("set upgrade_step 2", 0);
		run_command ("save", 0);
		into_recovery();
	}


	//added by Elvis for added fool idle
	get_key();
	get_key();
	
	while(hold_time > 0)
	{
		udelay(polling_time);
		tmp = get_key();
		printf("get_key(): %d\n", tmp);
		if(!tmp)  break;
		hold_time -= polling_time;
	}

	if(hold_time > 0)
	{
		printf("Normal Start...\n");
		return	1;
	}
	else
	{
		display_messge("upgrading... please wait");
		if(upgrade_bootloader())
		{
			run_command ("set upgrade_step 1", 0);
			run_command ("save", 0);
			run_command ("reset", 0);
			hang();
		}
		run_command ("set upgrade_step 2", 0);
		run_command ("save", 0);
		into_recovery();
	}


return	0;
}
#endif

#ifdef CONFIG_SARADC
#include <asm/saradc.h>
/*following key value are test with board 
  [OPLAY_AML8726_M_R100 : ASUS]
  ref doc:
  1. OPLAY_AML8726_M_R100_0817.pdf
  2. M3-Periphs-Registers.docx (Pg43-47)
*/
static struct adckey_info g_key_K1_info[] = {
    {"sw3", 0, 60}
};

static struct adc_info g_adc_info[] = {
    {"Press Key sw3", AML_ADC_CHAN_4, ADC_KEY,&g_key_K1_info},
    {"Press Key N/A",AML_ADC_CHAN_5, ADC_OTHER, NULL}
};

#ifdef CONFIG_KEYPRESS_TEST
#define KEY_DELAY_US 2000
void keypress_test(void)
{
  /*following is test code to test ADC & key pad*/
  int i;
	int count = sizeof(g_key_K1_info)/sizeof(struct adckey_info);
	int nKeyVal = 0;
	int nCnt = 0;
  printf("waiting for keypress test..\npress key \'sw3\' 3 times please...\n");
	saradc_enable();	
	while(nCnt < 3)
	{
		udelay(KEY_DELAY_US);
		nKeyVal = get_adc_sample(4);
		if(nKeyVal > 1000)
			continue;
		for(i = 0; i < count; i++){
			int v = g_key_K1_info[i].value;
			if(nKeyVal >= v && nKeyVal <= (v + g_key_K1_info[i].tolerance)){
				printf("key:%s pressed.\n",g_key_K1_info[i].key);
				break;
			}
		}
		if(i >= count)
			printf("unknown key pressed. keyvalue=%d\n",nKeyVal);
		nCnt++;
		for(i = 0; i < 100; i++)
			udelay(KEY_DELAY_US);
	}
	saradc_disable();
}
int do_keytest (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	keypress_test();
	return 0;
}

U_BOOT_CMD(
	keytest,	1,	1,	do_keytest,
	"test key press",
);
#endif /*CONFIG_KEYPRESS_TEST*/

#endif /*CONFIG_SARADC*/
