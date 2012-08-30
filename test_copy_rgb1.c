#include <includes.h>
#include <ioapi.h>
#include <displaydev.h>

#define MPEG_BASE_ADDR               0x01100000

#define WRITE_MPEG_REG(reg, val) (*(volatile unsigned *)(MPEG_BASE_ADDR + ((reg)<<2)) = (val))

#define READ_MPEG_REG(reg) (*(volatile unsigned *)(MPEG_BASE_ADDR + ((reg)<<2)))

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

#define BUFFER_SIZE 800*600*3

unsigned char* rgb_buff = 0;
video_screen_hardcopy_req_t m_sc;

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
    while (I8080_CONFIG_CMD & 1);   // wait till the busy bit became 0 
}

static void DisplayBuffer()
{
    register int i = 800*600;
    register int temp;
//    register unsigned char* p;
//    register unsigned char* maxp;
//    
//    p = rgb_buff;
//    maxp = rgb_buff + BUFFER_SIZE;
    
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
    	  while ((READ_MPEG_REG(VPP_OFIFO_SIZE) >> 9) == 0);
        temp = READ_MPEG_REG(LCD_MCU_DATA_1);
        
        while (I8080_CONFIG_CMD & 1);
        I8080_DATA_W = temp & 0xff;
        while (I8080_CONFIG_CMD & 1);
        I8080_DATA_W = temp >> 8;
          

	      // wait for VFIFO not empty
        //while ((READ_MPEG_REG(VPP_OFIFO_SIZE) >> 9) == 0);
        while (I8080_CONFIG_CMD & 1);
        I8080_DATA_W = READ_MPEG_REG(LCD_MCU_DATA_0) & 0xff;
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

static unsigned char m_flag = 0;

static void MyTask(void *arg)
{
    *(volatile unsigned*)0x12000b4 &= ~(1<<13);
    *(volatile unsigned*)0x12000c0 |= 1<<18;
    while(1)
    {
        if(m_flag)
        {
            m_flag = 0;
            memset(rgb_buff, 0, BUFFER_SIZE);
            GetRGB();
        }
        
        AVTimeDly(1000);
    }
    //AVTaskDel(my_task_id);
}


void Init_Test_Copy()
{
    rgb_buff = (unsigned char *)AVMem_malloc(BUFFER_SIZE);
    if(rgb_buff)
    {
        printf("\nMalloc memeory ok.\n");
     
        AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
    }
    else
        printf("\nMalloc memeory failed.\n");
}