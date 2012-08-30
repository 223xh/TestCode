#include <includes.h>
#include <ioapi.h>
#include <displaydev.h>
#include "chipsupport/chipsupport.h"
#include <amsysdef.h>
#include <interrupt.h>

#define IRQ_VERTSYNC (1<<3)

#define MPEG_BASE_ADDR               0x01100000

#define WRITE_MPEG_REG(reg, val) (*(volatile unsigned *)(MPEG_BASE_ADDR + ((reg)<<2)) = (val))

#define READ_MPEG_REG(reg) (*(volatile unsigned *)(MPEG_BASE_ADDR + ((reg)<<2)))

#define DELAY(a) for(m=0;m<a;m++)

#define I8080_CONFIG_CMD    *(volatile unsigned *)0x011013C0
#define I8080_CYCLE         *(volatile unsigned *)0x011013C4
#define I8080_START_1       *(volatile unsigned *)0x011013C8
#define I8080_END_1         *(volatile unsigned *)0x011013CC
#define I8080_START_2       *(volatile unsigned *)0x011013D0
#define I8080_END_2         *(volatile unsigned *)0x011013D4
#define I8080_DATA_RW       *(volatile unsigned *)0x011013D8
#define I8080_DATA_W        *(volatile unsigned *)0x011013DC

#define LCD_MCU_CTL             0x4dd
#define VPP_OFIFO_SIZE          0x827
#define LCD_MCU_DATA_0          0x4de
#define LCD_MCU_DATA_1          0x4df

#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

#define BUFFER_SIZE 240*320*3//800*600*3

static unsigned char* rgb_buff = 0;
static video_screen_hardcopy_req_t m_sc;

static int VsyncHandle = -1;

static void vsync_irq_handler(void *arg)
{
    *(volatile unsigned*)0x12000b0 |= 3<<15;//Enable sth1,cph1
    AVDetachIrq(VsyncHandle);
}

static void Write8Bit(INT8U data)
{
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
/*    
    I8080_CYCLE = 20;
    I8080_START_1 = 2;
    I8080_END_1 = 18;
    I8080_START_2 = 4;
    I8080_END_2 = 16;
    I8080_DATA_RW = 0;
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(2 << 10) |	// output_config_wr --- toggle_1
    				(0 <<  8 ) |	// output_config_rd --- not change
    				(1 <<  7 ) |	// dc set to 1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(1 <<  4 ) |	 // rd set to 1 
    				(0 <<  0 );	// this is not a command, it¡¯s just setting
*/    
    I8080_DATA_W = data;
    //while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0 
}

static void WriteI8080(INT8U m_data, INT8U m_type)
{
    if(m_type > 1)
        m_type = 1;
    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    
    I8080_CYCLE = 20;
    I8080_START_1 = 2;
    I8080_END_1 = 18;
    I8080_START_2 = 4;
    I8080_END_2 = 16;
    
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(2 << 10) |	// output_config_wr --- toggle_1
    				(0 <<  8 ) |	// output_config_rd --- toggle all cycle
    				(m_type <<  7 ) |	// dc  0 (command),1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(0 <<  4 ) |	// rd set to 0 (combine with output_config_rd, rd will be 1 all cycle)
    				(0 <<  0 );	// this is a write command
    				
    I8080_DATA_RW =  m_data;
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
}

static void Write8BitREG(INT8U m_data)
{    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    
    I8080_CYCLE = 20;
    I8080_START_1 = 2;
    I8080_END_1 = 18;
    I8080_START_2 = 4;
    I8080_END_2 = 16;
    I8080_DATA_RW =  0;
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(2 << 10) |	// output_config_wr --- toggle_1
    				(1 <<  8 ) |	// output_config_rd --- toggle all cycle
    				(0 <<  7 ) |	// dc  0 (command),1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(0 <<  4 ) |	// rd set to 0 (combine with output_config_rd, rd will be 1 all cycle)
    				(1 <<  0 );	// this is a write command
    
    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    I8080_DATA_RW =  m_data;
    I8080_CONFIG_CMD |= 1;
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
}

static void Write8BitData(INT16U m_data)
{    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    
    I8080_CYCLE = 20;
    I8080_START_1 = 2;
    I8080_END_1 = 18;
    I8080_START_2 = 4;
    I8080_END_2 = 16;
    I8080_DATA_RW = (m_data>>8) & 0xff;
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(2 << 10) |	// output_config_wr --- toggle_1
    				(1 <<  8 ) |	// output_config_rd --- toggle all cycle
    				(1 <<  7 ) |	// dc  0 (command),1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(0 <<  4 ) |	// rd set to 0 (combine with output_config_rd, rd will be 1 all cycle)
    				(1 <<  0 );	// this is a write command
    
    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    I8080_DATA_RW = m_data & 0xff;
    I8080_CONFIG_CMD |= 1;
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
}

static INT16U Read8Bit(void)
{
    INT16U rdata = 0;
    
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    
    I8080_CYCLE = 20;
    I8080_START_1 = 2;
    I8080_END_1 = 18;
    I8080_START_2 = 4;
    I8080_END_2 = 16;
    
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(0 << 10) |	// output_config_wr --- toggle_1
    				(2 <<  8 ) |	// output_config_rd --- toggle all cycle
    				(1 <<  7 ) |	// dc  0 (command),1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(1 <<  4 ) |	// rd set to 1
    				(3 <<  2 ) |	// use END_2 to latch data
    				(3 <<  0 );	// this is a read command
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    rdata = (I8080_DATA_RW & 0xff) << 8;
    I8080_CONFIG_CMD |= 1;
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
    rdata |= (I8080_DATA_RW & 0xff);
    
    return rdata;
}

//static unsigned d2 = 4;

static void DisplayBuffer()
{
    register int i = 240*320;//800*600;
    register int temp;
    //register int rgb;
    //register unsigned d1,d2;
    register unsigned m;
    
//    d1 = 0;
//    d2 = 5;
//    register unsigned char* p;
//    register unsigned char* maxp;
    
//    p = rgb_buff;
    
//    memset(rgb_buff, 0, BUFFER_SIZE);
//    maxp = rgb_buff + BUFFER_SIZE;
    while (I8080_CONFIG_CMD & 1);
    
    I8080_CYCLE = 10;//20;
    I8080_START_1 = 1;//2;
    I8080_END_1 = 9;//18;
    I8080_START_2 = 2;//4;
    I8080_END_2 = 8;//16;
    I8080_DATA_RW = 0;
    I8080_CONFIG_CMD     = 	(0 << 14) |	// output_config_dc --- not change
    				(3 << 12) |	// output_config_cs --- toggle_2
    				(2 << 10) |	// output_config_wr --- toggle_1
    				(0 <<  8 ) |	// output_config_rd --- not change
    				(1 <<  7 ) |	// dc set to 1 (data)
    				(1 <<  6 ) |	// cs set to 1
    				(1 <<  5 ) |	// wr set to 1
    				(1 <<  4 ) |	 // rd set to 1 
    				(0 <<  0 );	// this is not a command, it¡¯s just setting
    				
    WRITE_MPEG_REG(LCD_MCU_CTL,
                    (1 << 8) |  // vfifo_mcu_enable
                    (1 << 7) |  // halt_vs_de
                    (1 << 6) |  // R8G8B8_format
                    (0 << 5) |  // R6G6B6_format
                    (0 << 4) |  // R5G6B5_format
                    (1 << 3) |  // set dac_dith_sel
                    (1 << 0));  // set lcd_mcu_enable

    // wait for both lcd_mcu_enable_vsync and lcd_mcu_enable_de
    while ((READ_MPEG_REG(LCD_MCU_CTL) & 6) != 6);
    
    // wait for VFIFO not empty
    while ((READ_MPEG_REG(VPP_OFIFO_SIZE) >> 9) == 0);
	
    //while(p<maxp)
    while(i--)
    {        
		    //Write8Bit(*(p++));
		    //while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0
		    //if(I8080_CONFIG_CMD);
		    //I8080_DATA_W = *(p++);
		    
		    // wait for VFIFO not empty
    	  //while ((READ_MPEG_REG(VPP_OFIFO_SIZE) >> 9) == 0);
    	  //DELAY(d1);
        temp = READ_MPEG_REG(LCD_MCU_DATA_1);
/*        
        rgb = ((temp & 0xf8) << 8) | ((temp & 0xfc00) >> 5);
        //rgb = ((temp >> 3) & 0x1f) | ((temp & 0xfc00) >> 5);
        temp = READ_MPEG_REG(LCD_MCU_DATA_0) & 0xff;
        rgb |= ((temp >> 3) & 0x1f);
        //rgb |= ((temp & 0xf8) << 8);
        
        while (I8080_CONFIG_CMD & 1);
        I8080_DATA_W = (rgb >> 8) & 0xff;
        I8080_CONFIG_CMD |= 1;
        while (I8080_CONFIG_CMD & 1);
        I8080_DATA_W = rgb & 0xff;
        I8080_CONFIG_CMD |= 1;
*/

        
        while (I8080_CONFIG_CMD & 1);
        //DELAY(6);
        //I8080_DATA_W = temp & 0xff;
        I8080_DATA_W = temp & 0xff;
//        *p = temp & 0xff;
//        ++p;
//        I8080_CONFIG_CMD |= 1;
        while (I8080_CONFIG_CMD & 1);
        //DELAY(6);
        I8080_DATA_W = (temp >> 8) & 0xff;
//        *p = (temp >> 8) & 0xff;
//        ++p;
//        I8080_CONFIG_CMD |= 1;
          

	      // wait for VFIFO not empty
        //while ((READ_MPEG_REG(VPP_OFIFO_SIZE) >> 9) == 0);
        while (I8080_CONFIG_CMD & 1);
        //DELAY(6);
        //temp = READ_MPEG_REG(LCD_MCU_DATA_0) & 0xff;
        I8080_DATA_W = READ_MPEG_REG(LCD_MCU_DATA_0) & 0xff;
        //I8080_DATA_W = temp;
//        *p = temp;
//        ++p;
//        I8080_CONFIG_CMD |= 1;

    }
    
    WRITE_MPEG_REG(LCD_MCU_CTL, 0);
}

static void GetRGB()
{
    int vfd = -1;
    unsigned int len = 0;
    unsigned s,e;
    
/*    
    vfd = open("/dev/video", O_RDWR);
    if(vfd)
    {
        len = BUFFER_SIZE;
        m_sc.flag = VIDEO_SCFLAG_RESTART;
        m_sc.data = rgb_buff;
        m_sc.len = &len;
        
        s = AVTimeGet();
        ioctl(vfd, VIDEOIO_SCREEN_HARDCOPY, &m_sc);
        e = AVTimeGet();
        printf("\nCopy cost %ums.\n", e-s);
        close(vfd);
        
        s = AVTimeGet();
        DisplayBuffer();
        e = AVTimeGet();
        printf("\nDisplay cost %ums.\n", e-s);
    }
*/
        s = AVTimeGet();
        DisplayBuffer();
        e = AVTimeGet();
        printf("\nDisplay cost %ums.\n", e-s);
}

static void InitILI9325()
{
    printf("\nStart init 9325.\n");
    
    Write8BitREG(0xe3);
    Write8BitData(0x3008);
    Write8BitREG(0xe7);
    Write8BitData(0x0012);
    Write8BitREG(0xef);
    Write8BitData(0x1231);
    Write8BitREG(0x00);
    Write8BitData(0x0001);
    
    AVTimeDly(50);
    
    Write8BitREG(0x01);
    Write8BitData(0x0100);
    Write8BitREG(0x02);
    Write8BitData(0x0700);
    Write8BitREG(0x03);
    ////Write8BitData(0x1030);
    Write8BitData(0xc030);
    Write8BitREG(0x04);
    Write8BitData(0x0000);
    Write8BitREG(0x08);
    Write8BitData(0x0202);
    Write8BitREG(0x09);
    Write8BitData(0x0000);
    Write8BitREG(0x0a);
    ///Write8BitData(0x0000);
    Write8BitData(0x0008);
    Write8BitREG(0x0c);
    Write8BitData(0x0000);
    Write8BitREG(0x0d);
    Write8BitData(0x0000);
    Write8BitREG(0x0f);
    Write8BitData(0x0000);
    
    Write8BitREG(0x10);
    Write8BitData(0x0000);
    Write8BitREG(0x11);
    Write8BitData(0x0007);
    Write8BitREG(0x12);
    Write8BitData(0x0000);
    Write8BitREG(0x13);
    Write8BitData(0x0000);
    
    AVTimeDly(200);
    
    Write8BitREG(0x10);
    //Write8BitData(0x1690);
    ///Write8BitData(0x1290);
    Write8BitData(0x1490);
    Write8BitREG(0x11);
    Write8BitData(0x0227);
    
    AVTimeDly(100);
    
    Write8BitREG(0x12);
    //Write8BitData(0x001c);
    ///Write8BitData(0x0019);
    Write8BitData(0x001d);
    
    AVTimeDly(100);
    
    Write8BitREG(0x13);
    ///Write8BitData(0x1800);
    Write8BitData(0x1500);
    Write8BitREG(0x29);
    //Write8BitData(0x001c);
    ///Write8BitData(0x0027);
    Write8BitData(0x002d);
    Write8BitREG(0x2b);
    //Write8BitData(0x000d);
    Write8BitData(0x000b);
    
    AVTimeDly(100);
    
    Write8BitREG(0x20);
    Write8BitData(0x0000);
    Write8BitREG(0x21);
    Write8BitData(0x0000);
    
    Write8BitREG(0x30);
    //Write8BitData(0x0007);
    ///Write8BitData(0x0004);
    Write8BitData(0x0000);
    Write8BitREG(0x31);
    //Write8BitData(0x0302);
    ///Write8BitData(0x0506);
    Write8BitData(0x0307);
    Write8BitREG(0x32);
    ///Write8BitData(0x0105);
    Write8BitData(0x0101);
    Write8BitREG(0x35);
    //Write8BitData(0x0206);
    ///Write8BitData(0x0302);
    Write8BitData(0x0102);
    Write8BitREG(0x36);
    //Write8BitData(0x0808);
    ///Write8BitData(0x0004);
    Write8BitData(0x1406);
    Write8BitREG(0x37);
    //Write8BitData(0x0206);
    ///Write8BitData(0x0207);
    Write8BitData(0x0505);
    Write8BitREG(0x38);
    //Write8BitData(0x0504);
    ///Write8BitData(0x0102);
    Write8BitData(0x0003);
    Write8BitREG(0x39);
    //Write8BitData(0x0007);
    ///Write8BitData(0x0307);
    Write8BitData(0x0707);
    Write8BitREG(0x3c);
    //Write8BitData(0x0105);
    ///Write8BitData(0x0203);
    Write8BitData(0x0201);
    Write8BitREG(0x3d);
    //Write8BitData(0x0808);
    ///Write8BitData(0x0004);
    Write8BitData(0x1a06);
    
    Write8BitREG(0x50);
    Write8BitData(0x0000);
    Write8BitREG(0x51);
    Write8BitData(0x00ef);
    Write8BitREG(0x52);
    Write8BitData(0x0000);
    Write8BitREG(0x53);
    Write8BitData(0x013f);
    Write8BitREG(0x60);
    Write8BitData(0xa700);
    ///Write8BitData(0x2700);
    Write8BitREG(0x61);
    Write8BitData(0x0001);
    Write8BitREG(0x6a);
    Write8BitData(0x0000);
    
    Write8BitREG(0x80);
    Write8BitData(0x0000);
    Write8BitREG(0x81);
    Write8BitData(0x0000);
    Write8BitREG(0x82);
    Write8BitData(0x0000);
    Write8BitREG(0x83);
    Write8BitData(0x0000);
    Write8BitREG(0x84);
    Write8BitData(0x0000);
    Write8BitREG(0x85);
    Write8BitData(0x0000);
    
    Write8BitREG(0x90);
    Write8BitData(0x0010);
    Write8BitREG(0x92);
    Write8BitData(0x0000);
    ///Write8BitData(0x0600);
    Write8BitREG(0x93);
    Write8BitData(0x0003);
    Write8BitREG(0x95);
    Write8BitData(0x0110);
    Write8BitREG(0x97);
    Write8BitData(0x0000);
    Write8BitREG(0x98);
    Write8BitData(0x0000);
    
    Write8BitREG(0x07);
    Write8BitData(0x0133);
    
    printf("Init 9325 done.\n");
}

static unsigned char m_flag = 0;

static void MyTask(void *arg)
{
    INT16U tmp = 0;
    unsigned s,e;
    AVTimeDly(3000);
    
    *(volatile unsigned*)0x12000bc &= ~(3<<6);
    *(volatile unsigned*)0x12000bc &= ~(1<<23);
    *(volatile unsigned*)0x12000b0 &= ~(3<<30);
    *(volatile unsigned*)0x12000b0 &= ~(3<<9);
    //*(volatile unsigned*)0x12000b8 |= 0x1f<<16;
    *(volatile unsigned*)0x12000b8 |= 0xf<<16;
    
    *(volatile unsigned*)0x1200064 &= ~(1<<13);
    *(volatile unsigned*)0x1200060 &= ~(1<<13);
    
    //send a reset
    *(volatile unsigned*)0x12000b0 &= ~(1<<12);
    
    *(volatile unsigned*)0x1200034 |= (1<<5);
    *(volatile unsigned*)0x1200030 &= ~(1<<5);
    AVTimeDly(50);
    *(volatile unsigned*)0x1200034 &= ~(1<<5);
    AVTimeDly(100);
    *(volatile unsigned*)0x1200034 |= (1<<5);
    AVTimeDly(200);
    
    InitILI9325();


    Write8BitREG(0x3);
    tmp = Read8Bit();
    printf("Entry mode is 0x%x.\n", tmp);
    
//    Write8BitREG(0);
//    tmp = Read8Bit();
//    printf("Device code is 0x%x.\n", tmp);
//    
//    Write8BitREG(0x3);
//    tmp = Read8Bit();
//    printf("Entry mode is 0x%x.\n", tmp);
    
    Write8BitREG(0x22);
    #ifndef __ROM_
    printf("Start get rgb & send.\n");
    #endif
    while(1)
    {
        #ifndef __ROM_
        s = AVTimeGet();
        #endif
        DisplayBuffer();
        #ifndef __ROM_
        e = AVTimeGet();
        printf("\nDisplay cost %ums.\n", e-s);
        #endif
        
        AVTimeDly(100);
    }
    
}

#define TYPE_COMMAND 0
#define TYPE_DATA    1

#define OUTEN1    *(volatile unsigned *)0x01200060
#define OUTLV1    *(volatile unsigned *)0x01200064
#define INLV1     *(volatile unsigned *)0x01200068

#define OUTEN2    *(volatile unsigned *)0x01200030
#define OUTLV2    *(volatile unsigned *)0x01200034

#define DATA_PIN_MASK (0x3f3<<14)

#define WRB    1<<1//CPH1
#define RDB    1<<3//OEH    //1<<11
#define RSB    1<<4//STV1   //1<<12
#define CSB    1<<2//STH1

static void delay_10us(unsigned count)
{
    int i ;
    for(; count > 0 ; count--) 
      for(i=360; i > 0; i--) 
        {;}
}

static void WriteDataSW(INT16U m_data, INT8U m_type)
{
    OUTEN1 &= ~DATA_PIN_MASK;
    
    if(m_type)
        OUTLV2 |= RSB;       //RS=1
    else
        OUTLV2 &= ~(RSB);       //RS=0
        
    //OUTLV2 &= ~(CSB);       //CS=0
    OUTLV2 |= WRB;       //WR=1
    OUTLV1 &= ~DATA_PIN_MASK;    //Clear Data
    OUTLV1 |= (((m_data & 0xfc)<<16) | ((m_data & 3)<<14));  //Set Data
    
    OUTLV2 &= ~(WRB);       //WR=0
    delay_10us(1);
    OUTLV2 |= WRB;       //WR=1
    
    if(m_type)
        OUTLV2 &= ~(RSB);       //RS=0
    else
        OUTLV2 |= RSB;       //RS=1
        
    //OUTLV2 |= CSB;       //CS=1
    delay_10us(1);
}

static void WriteREG8BitSW(INT16U m_data)
{
    WriteDataSW(0, TYPE_COMMAND);
    WriteDataSW(m_data, TYPE_COMMAND);
}

static void WriteData8BitSW(INT16U m_data)
{
    WriteDataSW((m_data>>8)&0xff, TYPE_DATA);
    WriteDataSW(m_data&0xff, TYPE_DATA);
}

static INT16U ReadDataSW(void)
{
    unsigned odata;
    INT16U mdata;
    
    OUTEN1 |= DATA_PIN_MASK;
    
    OUTLV2 |= RSB;       //RS=1
        
    //OUTLV2 &= ~(CSB);       //CS=0
    OUTLV2 |= RDB;       //RD=1
    
    OUTLV2 &= ~(RDB);       //RD=0
    delay_10us(1);
    odata = INLV1 & DATA_PIN_MASK;
    OUTLV2 |= RDB;       //RD=1
    
    OUTLV2 &= ~(RSB);       //RS=0
        
    //OUTLV2 |= CSB;       //CS=1
    delay_10us(1);
    
    mdata = (((odata >> 16) & 0xfc) | ((odata >> 14) & 3)) << 8;
    
    //OUTLV2 &= ~(CSB);       //CS=0
    
    OUTLV2 &= ~(RDB);       //RD=0
    delay_10us(1);
    odata = INLV1 & DATA_PIN_MASK;
    OUTLV2 |= RDB;       //RD=1
    
    OUTLV2 &= ~(RSB);       //RS=0
        
    //OUTLV2 |= CSB;       //CS=1
    delay_10us(1);
    mdata |= (((odata >> 16) & 0xfc) | ((odata >> 14) & 3));
    
    return mdata;
}

static void SW_IO_INIT1()
{
    *(volatile unsigned*)0x12000b0 &= ~(0x1ff<<11);
    *(volatile unsigned*)0x1200034 |= (1<<6);//set high(8bit mode)
    *(volatile unsigned*)0x1200030 &= ~(1<<6);
    
    *(volatile unsigned*)0x1200034 |= (0xf<<1);//set cs, sb, rd, wr to high
    *(volatile unsigned*)0x1200030 &= ~(0xf<<1);
//    *(volatile unsigned*)0x1200064 |= 3<<11;
//    *(volatile unsigned*)0x1200060 &= ~(3<<11);
    
    OUTLV2 &= ~(CSB);//CS=0
}

static void SW_IO_INIT2()
{
    *(volatile unsigned*)0x1200034 &= ~(1<<6);//set low(16bit mode)
    
    OUTLV2 |= RDB;       //RD=1
    OUTLV2 |= RSB;       //RS=1
    *(volatile unsigned*)0x1102d84 |= 1<<1;//Invert sth1
    *(volatile unsigned*)0x12000b0 |= 1<<9;//Set RGB 6bit output
    //*(volatile unsigned*)0x12000b0 |= 3<<15;//Enable sth1,cph1
//    if(VsyncHandle < 0)
//        VsyncHandle = AVRequestIrq(ISR_TYPE_GENERAL, IRQ_VERTSYNC, vsync_irq_handler, NULL);
}

#define GET_STH_LEVEL ((*(volatile unsigned*)0x1200038)&(1<<2))

void CheckFrameHead()
{    
    unsigned char flag = 0;
    unsigned l_count, h_count;
    
    l_count = h_count = 0;
    
    *(volatile unsigned*)0x1101304 |= (1<<11);//Set sth1 to line signal
    
    *(volatile unsigned*)0x12000b0 |= 1<<15;//Enable sth1;
    
    delay_10us(10);
    while(1)
    {
        if(!flag)
        {
            if(!GET_STH_LEVEL)
                ++l_count;
            else
                l_count = 0;
                
            if(l_count >= 3)
            {
                flag = 1;
            }
        }
        else if(flag == 1)
        {
            if(GET_STH_LEVEL)
                ++h_count;
            else
            {
                flag = 0;
                h_count = 0;
            }
            
            if(h_count >= 3)
            {
                *(volatile unsigned*)0x1101304 &= ~(1<<11);
                delay_10us(1);
                *(volatile unsigned*)0x12000b0 |= 1<<16;//Enable cph1;
                break;
            }
        }
        
        delay_10us(10);
    }
}

static void InitILI9325_SW()
{
    SW_IO_INIT1();
    printf("\nStart init 9325(gpio mode).\n");
    
    WriteREG8BitSW(0xe3);
    WriteData8BitSW(0x3008);
    WriteREG8BitSW(0xe7);
    WriteData8BitSW(0x0012);
    WriteREG8BitSW(0xef);
    WriteData8BitSW(0x1231);
    WriteREG8BitSW(0x00);
    WriteData8BitSW(0x0001);
    
    AVTimeDly(50);
    
    WriteREG8BitSW(0x01);
    WriteData8BitSW(0x0100);
    WriteREG8BitSW(0x02);
    WriteData8BitSW(0x0700);
    WriteREG8BitSW(0x03);
    WriteData8BitSW(0x0030);
    WriteREG8BitSW(0x04);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x08);
    WriteData8BitSW(0x0202);
    WriteREG8BitSW(0x09);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x0a);
    WriteData8BitSW(0x0008);
    WriteREG8BitSW(0x0c);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x0d);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x0f);
    WriteData8BitSW(0x0000);
    
    WriteREG8BitSW(0x10);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x11);
    WriteData8BitSW(0x0007);
    WriteREG8BitSW(0x12);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x13);
    WriteData8BitSW(0x0000);
    
    AVTimeDly(200);
    
    WriteREG8BitSW(0x10);
    WriteData8BitSW(0x1490);
    WriteREG8BitSW(0x11);
    WriteData8BitSW(0x0227);
    
    AVTimeDly(100);
    
    WriteREG8BitSW(0x12);
    WriteData8BitSW(0x001d);
    
    AVTimeDly(100);
    
    WriteREG8BitSW(0x13);
    WriteData8BitSW(0x1500);
    WriteREG8BitSW(0x29);
    WriteData8BitSW(0x002d);
    WriteREG8BitSW(0x2b);
    WriteData8BitSW(0x000b);
    
    AVTimeDly(100);
    
    WriteREG8BitSW(0x20);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x21);
    WriteData8BitSW(0x0000);
    
    WriteREG8BitSW(0x30);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x31);
    WriteData8BitSW(0x0307);
    WriteREG8BitSW(0x32);
    WriteData8BitSW(0x0101);
    WriteREG8BitSW(0x35);
    WriteData8BitSW(0x0102);
    WriteREG8BitSW(0x36);
    WriteData8BitSW(0x1406);
    WriteREG8BitSW(0x37);
    WriteData8BitSW(0x0505);
    WriteREG8BitSW(0x38);
    WriteData8BitSW(0x0003);
    WriteREG8BitSW(0x39);
    WriteData8BitSW(0x0707);
    WriteREG8BitSW(0x3c);
    WriteData8BitSW(0x0201);
    WriteREG8BitSW(0x3d);
    WriteData8BitSW(0x1a06);
    
    WriteREG8BitSW(0x50);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x51);
    WriteData8BitSW(0x00ef);
    WriteREG8BitSW(0x52);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x53);
    WriteData8BitSW(0x013f);
    WriteREG8BitSW(0x60);
    WriteData8BitSW(0xa700);
    WriteREG8BitSW(0x61);
    WriteData8BitSW(0x0001);
    WriteREG8BitSW(0x6a);
    WriteData8BitSW(0x0000);
    
    WriteREG8BitSW(0x80);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x81);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x82);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x83);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x84);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x85);
    WriteData8BitSW(0x0000);
    
    WriteREG8BitSW(0x90);
    WriteData8BitSW(0x0010);
    WriteREG8BitSW(0x92);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x93);
    WriteData8BitSW(0x0003);
    WriteREG8BitSW(0x95);
    WriteData8BitSW(0x0110);
    WriteREG8BitSW(0x97);
    WriteData8BitSW(0x0000);
    WriteREG8BitSW(0x98);
    WriteData8BitSW(0x0000);
    
    WriteREG8BitSW(0x07);
    WriteData8BitSW(0x0133);
    
    printf("Init 9325 done.\n");
}

static void TCONMCU(void)
{
    INT16U tmp = 0;
    unsigned s,e;
    AVTimeDly(3000);
    
    *(volatile unsigned*)0x12000bc &= ~(3<<6);
    *(volatile unsigned*)0x12000bc &= ~(1<<23);
    *(volatile unsigned*)0x12000b0 &= ~(3<<30);
    *(volatile unsigned*)0x12000b0 &= ~(3<<9);
    
    //send a reset
    *(volatile unsigned*)0x12000b0 &= ~(1<<12);
    
    *(volatile unsigned*)0x1200034 |= (1<<5);
    *(volatile unsigned*)0x1200030 &= ~(1<<5);
    AVTimeDly(50);
    *(volatile unsigned*)0x1200034 &= ~(1<<5);
    AVTimeDly(100);
    *(volatile unsigned*)0x1200034 |= (1<<5);
    AVTimeDly(200);
    
    InitILI9325_SW();
    
    WriteREG8BitSW(0x0);
    tmp = ReadDataSW();
    printf("Device code is 0x%x.\n", tmp);

    WriteREG8BitSW(0x3);
    tmp = ReadDataSW();
    printf("Entry mode is 0x%x.\n", tmp);
    
    WriteREG8BitSW(0x22);  
    
    SW_IO_INIT2();
}


void Init_Test_Copy()
{
//    rgb_buff = (unsigned char *)AVMem_malloc(BUFFER_SIZE);
//    if(rgb_buff)
//    {
//        printf("\nMalloc memeory ok.\n");
//     
//        AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
//    }
//    else
//        printf("\nMalloc memeory failed.\n");
    //AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
    TCONMCU();
}