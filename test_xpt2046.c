#include <includes.h>

#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

#define rd(reg) (*(volatile unsigned *)(reg))

static unsigned user_cmd = (1<<18)|(0<<12)|(1<<13);
static unsigned xpt_cmd = 0xd7;//0xf7;//0xf3; //0x83
//static unsigned user_addr = 0xa5a5;

static unsigned send_cmd()
{
    unsigned data;
    
    //*(volatile unsigned*)0x1200504 = (2<<24)|user_addr;
    *(volatile unsigned*)0x1200504 = (2<<24);
    *(volatile unsigned*)0x1200500 = user_cmd | xpt_cmd;
    while(*(volatile unsigned*)0x1200500 & (1<<18));
    
    data = *(volatile unsigned*)0x1200520;
    
    return data;
}

static void MyTask(void *arg)
{
    unsigned m_flag = 1;
//    unsigned char m_cmd1 = 0x83;
//    unsigned char m_cmd2 = 0xf3;
//    unsigned data1 = 0;
//    unsigned data2 = 0;
    unsigned data, od;
    OS_CPU_SR  cpu_sr;
    AVTimeDly(5000);
    *(volatile unsigned*)0x1200508 = 0xaaf7f;//0xaa525;//sysclk/6 //0xaaf7f;//sysclk/16
    *(volatile unsigned*)0x12000b0 |= (0xf<<2);//disable all spi
    while(1)
    {
        if(m_flag)
        {
            m_flag = 0;
//            data1 = 0;
//            data2 = 0;
            data = 0;
            od = 0;
            OS_ENTER_CRITICAL();
            rd(0x1100574) &= ~(1<<8);//set system clock connect to 27Mhz crystal
            
            *(volatile unsigned*)0x12000b0 |= 1<<1;//disable nand
            *(volatile unsigned*)0x12000b0 &= ~(1<<0);//disable nand pinmux
            *(volatile unsigned*)0x12000b0 &= ~(1<<4);//SPI_D_A, SPI_Q_A, SPI_CS_n_A and SPI_C_A
            *(volatile unsigned*)0x12000b0 |= 1<<14;//CS
//            data1 = send_cmd(m_cmd1);
//            data2 = send_cmd(m_cmd2);
            data = send_cmd();
            *(volatile unsigned*)0x12000b0 &= ~(1<<14);
            *(volatile unsigned*)0x12000b0 |= 1<<4;
            *(volatile unsigned*)0x12000b0 |= 1<<0;
            *(volatile unsigned*)0x12000b0 &= ~(1<<1);
            
            rd(0x1100574) |= (1<<8);
            OS_EXIT_CRITICAL();
            
            od = data&0xffff;
            //od = ((od&0x3f)<<6) | (od>>10);
            od = ((od&0x7f)<<5) | (od>>11);
            printf("xpt_cmd=0x%x, data=0x%x, od=0x%x\n", xpt_cmd, data, od);
        }
        AVTimeDly(500);
    }
}

void Init_Test_XPT2046()
{
    AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
}