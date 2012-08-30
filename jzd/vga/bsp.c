/*******************************************************************
 * 
 *  Copyright C 2005 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description:  BSP for Amlogic 3510 LAN91C111+USB 2 SDRAM dev board.
 *
 *  Author: Amlogic Software
 *  Created: Wed Nov 23 15:18:47 2005
 *
 *******************************************************************/

#include <includes.h>
#include "bsp.h"
#include "ref_apollo_io.h"
#include <apollo/clock.h>
#include <apollo/ddrmem.h>
#include <apollo/video_setting.h>
#include <apollo/gpio.h>
#include <apollo/pinmux.h>
#include <apollo/usb_setting.h>
#include <memmap.h>
#include <card_io.h>
#include <drivers/block/spi/spi_flash.h>

static unsigned char *sys_base;   /* Base address of the ARC's memory */
static unsigned char *sys_extent; /* Max address of the ARC's memory */
extern unsigned long _reset;

#ifdef APOLLO_PMP_LCD
static void lcd_init(void);
static void InitIO();
#endif
void MarkResetInfo(void);
void InitKeyboard(void);
#define LCD_800X480_INNOLUX_AT070TN92


/**
 * Retrieve the base address for the system DRAM.
 * bsp_init() must be called first.
 */
unsigned char *
sys_base_address(void)
{
    return sys_base;
}

/**
 * Retrieve the maximum address for the system DRAM.
 * bsp_init() must be called first.
 */
unsigned char *
sys_max_address(void)
{
    return sys_base;
}

/**
 * Control USB VBus Power by GPIO Line.
 * bsp_init() must be called first.
 * @param [in] is_power_on TRUE of Power on VBus, FALSE of Power off VBus
 */
void bsp_set_dwc_chip_0_gpio_power(char is_power_on)
{
	if(is_power_on)
	{
		wr_reg(PREG_GPIO_CARD_OE,rd_reg(PREG_GPIO_CARD_OE) & (~(1<<25)));
		wr_reg(PREG_GPIO_CARD_OUTLVL,rd_reg(PREG_GPIO_CARD_OUTLVL) | (1<<25));
		
	}
	else
	{
		wr_reg(PREG_GPIO_CARD_OE,rd_reg(PREG_GPIO_CARD_OE) & (~(1<<25)));
		wr_reg(PREG_GPIO_CARD_OUTLVL,rd_reg(PREG_GPIO_CARD_OUTLVL) & (~(1<<25)));
		
	}
}

#ifdef NAND_USE_XD_IO
void nand_io_enable(void)
{
    /**
        XD_RE       ----pin250(CARD6)
        XD_WE       ----pin249(CARD7)
        XD_READY    ----pin248(CARD8)
        XD_ALE      ----pin247(CARD9)
        XD_CLE      ----pin246(CARD10)
        XD_CE       ----pin235(CARD17)
    **/    
//    *(volatile unsigned *) 0xc12000BC |= (0xf7<<20);
    wr_reg(PREG_PIN_MUX_REG3, rd_reg(PREG_PIN_MUX_REG3) | (0xf7<<20));
   
}
#endif
/**
 * Set up memory and PLLs, etc specific to this board.
 */
void SwitchLanguage(INT8U language_id)
{
	char * uni_charset[]=
	{
		"GB2312",
		"iso8859-1",
		"iso8859-2",
		"iso8859-5",
		"iso8859-7",
		"iso8859-9",
		"cp949",
		"BIG5",
		"cp1251",
		"KOI8-R",
		"KOI8-U/RU",
		"cp932",
		"Unicode"
	};

unsigned char lang2charset[]=
	{
		1,					//engligsh -- 8859-1
		0,					//gb2312
		2,
		3,
		4,
		5,
		6,
		7,					//big5
		8,					//cp1251
		9,					//KOI8-R
		10,					//KOI8-U/RU
		11,					//cp932
		12					//Unicode
	};
register_all_nls_tables();
	init_nls_table(uni_charset[lang2charset[language_id]]);
}	

extern _phymem_node_t MI_BUFFER[];
extern _phymem_node_t PROG_BUFFER[] ;

void
bsp_init(void)
{
    int ddr_phy_cntl_reg, N, M;
    MarkResetInfo();
    //add by zhenfei li
    //get the reset info which is marked in the memory before the system reset;
    _phymem_node_t *memmap = Am_GetSystemMem(MEMMAP_DEFAULT, MEMITEM_SYS_DRAM);
	//add by zhouzhi
	//reset all the bank to clear the last data..
	//add end
    sys_base = (unsigned char *) memmap->start;
    sys_extent = (unsigned char *) memmap->end;
    
//    we have set it in boot loader
//    ddrmem_config(DDRCFG_32Mx16, 16, DDRCLK_27_200);
    //set_sysclk(SYS_CLOCK_1560);
    
    //set ddr to 192M
    /*
    ddr_phy_cntl_reg = *((volatile unsigned *)(0xc1100000+0x1382*4));
    N = (ddr_phy_cntl_reg&0x3e00)>>9;
    M = 192*N/24;
    ddr_phy_cntl_reg = (ddr_phy_cntl_reg&0xfffffe00)|M;
    *((volatile unsigned *)(0xc1100000+0x1382*4)) = ddr_phy_cntl_reg;
    */
    set_sysclk_ex(3840);
    //set_cpu1clk_x2(1);
 
    // set video dac output
	//Disable DVIN, BT656, HDMI
	clear_mio_mux(1, (1<<29) | (1<<30) | (1<<31));
	clear_mio_mux(3, (1<<23) | (1<<24) | (1<<26));
	clear_mio_mux(4, (0x3f<<26));
	
#ifndef HDMI_OUTPUT	
	//enable Tcon
	//set_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7) | (1<<8) | (1<<9));
	set_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7));
	//enable RGB data
	//set_mio_mux(4, (1<<20) | (1<<22) | (1<<24));//6bits
	set_mio_mux(4, (0x3f<<20));//8bits
#else
    set_mio_mux(4, 0x7c000000);       //mux GPIOC_28 to hsync, mux GPIOC_29 to vsync, mux GPIOC_30 to DE, mux GPIOC_31 to pixel clock, mux GPIOC_32~GPIOC_55 to HDMI data lines,   
#endif	
    set_mio_mux(2, 0x40000);   //mux CARD0 to SPDIF output
#ifdef APOLLO_PMP_LCD	
	InitIO();
#endif
	
 	Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_MI_BUFFER, MI_BUFFER); 
 	Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_PROG_JPEG, PROG_BUFFER); 
//for ts file 
    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_DMX_DRAM, DEFAULT_DMX_MEMORY);
    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_DMX_DRAM2, DEFAULT_DMX_MEMORY2);
    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_DMX_SECTION_DRAM, DEFAULT_DMX_SECTION_BUF);
    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_DMX_SECTION_DRAM2, DEFAULT_DMX_SECTION_BUF2);
//for ts file

 	pinmux_audio_gpioa();	// for external audio dac
 	//set TEST_n to gpio for sd_en
    wr_reg(PREG_PIN_MUX_REG3,rd_reg(PREG_PIN_MUX_REG3) & (~(1<<0)));
    //enable gpioa bit 4
    wr_reg(PREG_PIN_MUX_REG3,rd_reg(PREG_PIN_MUX_REG3) & (~(1<<5)));
 	
	set_usba_phy_config(USE_EXT_CLOCK);  
#ifdef ENABLE_CARD_READER		

    card_module_enable(CARD_MODULE_SD_MMC);
#ifndef APOLLO_6826_32M	
	card_module_enable(CARD_MODULE_CF);
    card_module_enable(CARD_MODULE_MS_MSPRO);
    card_module_enable(CARD_MODULE_XD);
    set_ata_eight_bit(1);		
#endif 
		
#endif 
#ifdef ENABLE_SPIF_FLASH   
    SPISupport(&spi_w25x16);
    SPISupport(&spi_w25q16);
    SPISupport(&spi_mx25l3205d);
    SPISupport(&spi_w25x32);
    SPISupport(&spi_w25x16V);
    SPISupport(&spi_MX25L800);
    SPISupport(&spi_en25b32b);
    SPISupport(&spi_MX25L160);
    SPISupport(&spi_MX25X20);
    SPISupport(&spi_w25x40);
#endif	
//	    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_IP_PBUF    , DEFAULT_IP_PBUF);
	    //Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_DMA_DRAM, DEFAULT_DMA_MEMORY);
//add by zhouzhi
//for Ethernet DMA memory; move from lib;
    Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_ETHERNET_DMA_DRAM            ,ETHERNET_DMA_BUF);

#ifdef SWF_SUPPORT
    Am_RegisterMemType(MEMMAP_DEFAULT,MEMITEM_SWF_POOL1,     DEFAULT_SWF_POOL1);
    Am_RegisterMemType(MEMMAP_DEFAULT,MEMITEM_SWF_POOL2,     DEFAULT_SWF_POOL2);
    Am_RegisterMemType(MEMMAP_DEFAULT,MEMITEM_SWF_POOL3,     DEFAULT_SWF_POOL3);	
    Am_RegisterMemType(MEMMAP_DEFAULT,MEMITEM_SWF_CANVAS,    DEFAULT_SWF_CANVAS);
#else
    //Am_RegisterMemType(MEMMAP_DEFAULT, MEMITEM_IP_PBUF    , DEFAULT_IP_PBUF);		
#endif       
	set_int_vector_base((unsigned)&_reset);
	init_system_gpio();
    SwitchLanguage(1);
#ifdef APOLLO_PMP_LCD    
    lcd_init();
    //setting the video decoder output with 50hz
    policy_set_lcd_freq(45);
#endif
    policy_set_ac3_digiout_pcm(1);
#ifdef __ROM_
    policy_set_h264_decode_buf(0x2700000);
#endif
    policy_set_video_half_resolution(1);
    
    InitKeyboard();
    
    set_powerdetect_enable();
    set_powerhold_enable();
    clear_mio_mux(3,1<<0); //enable TEST_N to GPIO mode
    PowerOnCheck();
    *(volatile unsigned*)0xc1108048 &= ~(1<<8);
    *(volatile unsigned*)0xc110804c |=  (1<<8);
}

// please remove the following function from aw_txt_app.c
unsigned upgrade_buf_define[10][2]=
{
  {0,0},
};

unsigned char *AWTXTGetFixedMem()
{
	return 0;
}

#ifdef LCD_800X480_INNOLUX_AT070TN92
#define LCD_WIDTH       800
#define LCD_HEIGHT      480
#define MAX_WIDTH       1056
#define MAX_HEIGHT      525
#define VIDEO_ON_LINE   17

//Vcom:3.41V
//0.25 0.92 2.88 6.27 11.17 17.26 25.09 34.92 46.56 59.52 74.02 89.69 105.90 122.90 138.60 155.80 172.50 190.50 
void generate_gamma_table_dlCD(void)
{
    int i;
    int gamma_adjust[256] =    
    {
				0,0,0,1,2,2,3,4,5,6,6,7,8,9,10,11,
				11,12,13,14,15,16,17,18,18,19,20,21,22,23,24,25,
				26,26,27,28,29,30,31,32,33,34,35,35,36,37,38,39,
				40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,53,
				54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,
				70,71,72,72,73,74,75,76,77,78,79,80,81,82,83,84,
				85,86,87,88,89,89,90,91,92,93,94,95,96,97,98,99,
				99,100,101,102,103,104,105,106,107,108,108,109,110,111,112,113,
				114,115,116,117,118,118,119,120,121,122,123,124,125,126,127,128,
				129,130,131,132,133,133,134,135,136,137,138,139,140,141,142,143,
				144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
				159,160,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
				176,177,178,179,180,181,182,184,185,186,187,188,189,190,191,192,
				193,195,196,197,198,200,201,202,204,205,206,207,209,210,211,212,
				214,215,216,217,219,220,221,222,224,225,226,228,229,230,232,233,
				234,236,237,238,240,241,243,244,245,247,248,249,251,252,253,255
		};
		for(i=0;i<256;i++)
		{
			GammaTableR[i]=gamma_adjust[i]<<2;
			GammaTableG[i]=gamma_adjust[i]<<2;
			GammaTableB[i]=gamma_adjust[i]<<2;
		}
}

static lcdConfig_t lcd_config = {
    /* width & height */
    640, 480,
    /* max width & max height */
    800, 525,
    /* video_on_line */
    17,  

    /* pll_ctrl/clk_ctrl*/
    0x626, 0x1fc1,   //25.2MHz,60Hz,24MHz
    /* gamma_cntl_port */
    (0 << LCD_GAMMA_EN) | (0 << LCD_GAMMA_RVS_OUT) | (1 << LCD_GAMMA_VCOM_POL),
    /* gamma_vcom_hswitch_addr */
    0,
    /* rgb_base_addr */
    0xf0,
    /* rgb_coeff_addr */
    0x74a,        
    /* pol_cntl_addr */
    (0x0 << LCD_CPH1_POL) |(0x1 << LCD_HS_POL) | (0x1 << LCD_VS_POL), 
    /* dith_cntl_addr */
    0x400,      
    /* STH1 hs/he/vs/ve */     //STH
    19, 120, 0, 525-1,		//0, 0, 0, 0,
    /* STH2 hs/he/vs/ve */
    0, 0, 0, 0, 
    /* OEH hs/he/vs/ve */
    64, 799, 17, 524,
    /* vcom hswitch_addr/vs/ve */
    0, 0, 0,
    /* CPV1 hs/he/vs/ve */
    0, 0, 0, 0,
    /* CPV2 hs/he/vs/ve */  
    0, 0, 0, 0,    
    /* STV1 hs/he/vs/ve */
    0, 800 - 1, 6, 4,//0, 0, 0, 0,
    /* STV2 hs/he/vs/ve */
    0, 0, 0, 0,
    /* OEV1 hs/he/vs/ve */
    0, 0, 0, 0,
    /* OEV2 hs/he/vs/ve */     
    0, 0, 0, 0,
    /* OEV3 hs/he/vs/ve */    
    0, 0, 0, 0, 
    /* inv_cnt_addr */
    (0 << LCD_INV_EN)|(0 << LCD_INV_CNT),  
    /* tcon_misc_sel_addr */
    (1 << LCD_STV1_SEL)|(1 << LCD_STV2_SEL),
    /* dual_port_cntl_addr */
    (0x0<<LCD_RGB_SWP)|(0x1 << LCD_TTL_SEL)|(0x1 << LCD_ANALOG_SEL_CPH3)|(0x1 << LCD_ANALOG_3PHI_CLK_SEL), 
    LCD_DIGITAL,
		4,3
};
#endif

void static my_set_pwm0(unsigned lo, unsigned hi)
{
    wr_reg(LCD_PWM0_LO_ADDR, lo);
    wr_reg(LCD_PWM0_HI_ADDR, hi);
}

//  156M(System Clock)/600K=260
#define PWM_MAX 260
void lcd_backlight_brightness(unsigned level)
{
#if 1
	level += 1;
    unsigned diff;
    level = level>6?6:level;
    switch(level)
    {
		    //case 6://most bright
		       // my_set_pwm0(PWM_MAX, 0);
		        break;
		    case 0://most dark		        
		        my_set_pwm0(0, PWM_MAX);
		        break;
		    default:
		        diff = (unsigned)(((float)PWM_MAX)/6*(6-level)*0.4);
			if (level == 1)			// 0
				diff = 150;	
			else if (level == 2)		// 1
				diff = 130;//140;	
			else if (level == 3)		// 2
				diff = 120;//130;	
			else if (level == 4)		// 3
				diff = 100;//120;
			else if (level == 5)		// 4
				diff = 80;//100;
			else if (level == 6)		// 5
				diff = 60;//75;
		        my_set_pwm0(PWM_MAX - diff, 0 + diff);
		        break;
    }
#else
    unsigned diff;
    level = level>5?5:level;
    switch(level)
    {
		    case 5://most bright
		        my_set_pwm0(PWM_MAX, 0);
		        break;
		    case 0://most dark		        
		        my_set_pwm0(0, PWM_MAX);
		        break;
		    default:
		        diff = (unsigned)(((float)PWM_MAX)/5*(5-level)*0.4);
		        my_set_pwm0(PWM_MAX - diff, 0 + diff);
		        break;
    }

#endif
}

#define C_TIMER_MUX (*(volatile unsigned *)0xc1109940)
#define C_TIMER_E (*(volatile unsigned *)0xc1109954)

static void delay_ms(unsigned ms)
{
    unsigned val;
    
    val = ((C_TIMER_MUX>>8) & 3);
    if(val == 1)
        ms = ms * 1000;
    else if(val == 2)
        ms = ms * 100;
    else if(val == 3)
        ms = ms * 10;
        
    C_TIMER_E = 0;
    while(C_TIMER_E < ms);
}

void PowerOnSequence()
{
#ifdef LCD_800X480_TPO
    rd_reg(PREG_GPIOC_OUTLVL) |= (1<<8);//GPIOC8(+-5V Control) for 1.2
#endif
    set_mio_mux(5, (1<<4));//set CLK   
    rd_reg(PREG_GPIOB_OUTLVL) |= (1<<19);//GPIOB19(STBY) PULL HIGH
    
    delay_ms(2);
    
    rd_reg(PREG_GPIOC_OUTLVL) |= (1<<23);//GPIOC23(GRESET/STH2) PULL HIGH    
    set_mio_mux(5, (1<<0) | (1<<2) | (1<<3));//set tcon
    
    delay_ms(120);
    
    set_mio_mux(4, (0x3f<<20));//enable 8bits rgb data
    rd_reg(PREG_GPIOC_OUTLVL) |= (1<<8);//GPIOC8(+-5V Control) for 1.2
}

void PowerOffSequence()
{
    clear_mio_mux(4, (0x3f<<20));//disable 8bits rgb data
    
    delay_ms(180);
    
    clear_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4));//clear tcon & CLK
    
    delay_ms(2);
    
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<8);//GPIOC8(+-5V Control) for 1.2
}

void power_off_backlight(void)
{
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<10);//GPIOC10(BL_EN)
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<23);//GPIOC23(GRESET/STH2)
    rd_reg(PREG_GPIOB_OUTLVL) &= ~(1<<19);//GPIOB19(STBY)
    rd_reg(PREG_PIN_MUX_REG5) &= ~(1<<15);//GPIOC22(PWM Gate)
    PowerOffSequence();
}

void power_on_backlight(void)
{
    PowerOnSequence();
    rd_reg(PREG_PIN_MUX_REG5) |= (1<<15);//GPIOC22(PWM Gate)
    delay_ms(200);
    rd_reg(PREG_GPIOC_OUTLVL) |= (1<<10);//GPIOC10(BL_EN)
}

#ifdef APOLLO_PMP_LCD

static void InitIO()
{
    rd_reg(PREG_PIN_MUX_REG0) &= ~(1<<27);//GPIOC23(GRESET/STH2)
    rd_reg(PREG_PIN_MUX_REG2) &= ~(1<<22);
    rd_reg(PREG_PIN_MUX_REG5) &= ~(3<<13);    
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<23);//Set to low
    rd_reg(PREG_GPIOC_OE) &= ~(1<<23);
    
    rd_reg(PREG_PIN_MUX_REG0) &= ~(0xf<<5);//GPIOB19(STBY)
    rd_reg(PREG_GPIOB_OUTLVL) &= ~(1<<19);//Set to low
    rd_reg(PREG_GPIOB_OE) &= ~(1<<19);    
    
    rd_reg(PREG_PIN_MUX_REG0) &= ~(1<<26);//GPIOC10(BL_EN)
    rd_reg(PREG_PIN_MUX_REG2) &= ~(1<<16);
    rd_reg(PREG_PIN_MUX_REG1) &= ~(1<<1);
    rd_reg(PREG_PIN_MUX_REG6) &= ~(1<<6);
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<10);
    rd_reg(PREG_GPIOC_OE) &= ~(1<<10);
    
    rd_reg(PREG_PIN_MUX_REG0) &= ~(1<<27);//GPIOC22(BL_PWM)
    rd_reg(PREG_PIN_MUX_REG5) &= ~(1<<15);
    
    rd_reg(PREG_PIN_MUX_REG6) &= ~(1<<8);//GPIOC8(+-5V Control) for 1.2
    rd_reg(PREG_GPIOC_OUTLVL) &= ~(1<<8);
    rd_reg(PREG_GPIOC_OE) &= ~(1<<8);
    rd_reg(PREG_PIN_MUX_REG2) &= ~(1<<2); //CARD16
}


//For TPO4.3' LCD

static void lcd_init(void)
{
    set_lcd_config(&lcd_config);
    generate_gamma_table_dlCD();
    
    my_set_pwm0(PWM_MAX, 0);
    //power_on_backlight();
}
#endif

void InitKeyboard(void)
{
//pinmux_audio_gpioa wrong set the two gpio for external audio dac
   // wr_reg(PREG_PIN_MUX_REG3,rd_reg(PREG_PIN_MUX_REG3) & (~(1<<6)));
    //wr_reg(PREG_PIN_MUX_REG3,rd_reg(PREG_PIN_MUX_REG3) & (~(1<<7)));
    
     wr_reg(PREG_GPIOA_OE, rd_reg(PREG_GPIOA_OE) | (0xc3<<5));
     wr_reg(PREG_GPIO_CARD_OE, rd_reg(PREG_GPIO_CARD_OE) | (0x7<<22));
}

unsigned bsp_reset_flag = 0;
unsigned boot_code_end_flag;

static void MarkResetInfo(void)
{
    if(*((volatile unsigned *)0x81000004) == 0x55aa55aa) //soft_reset;
       bsp_reset_flag = 1;
}
void thunder_mgr_set_kankan_disk()
{
}
void http_file_open()
{}
void http_file_get_data()
{}
void http_file_get_length()
{}
void http_file_close()
{}

