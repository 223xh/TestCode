#include <includes.h>		
#include <af_engine.h>			
#include <af_message.h>		
#include <af_msgtype.h>		
#include <af_global.h>
#include <drivers/serial/uart.h>

void InitSerial(void);
void InitSerialTool(void);

#define BAUD_SET_NUM 115200
#define BAUD_SET_MARCO(b) BAUD_RATE_##b
#define BAUD_SET_MARCO_P(a) BAUD_SET_MARCO(a)

#define TASK_STK_SIZE 1024
OS_STK TaskStartStkSerialTask[TASK_STK_SIZE];
OS_STK TaskStartStkSerialToolTask[TASK_STK_SIZE];
unsigned short serial_task_id = 136;
unsigned short serial_tool_task_id = 138;
int fd_serial = -1;

static void write_serial(unsigned char * buffer, unsigned length)
{
    if(fd_serial > 0)
    {        
        write(fd_serial, buffer, length);
    }    
}

static void write_serial_byte(unsigned char *buff)
{
    if(fd_serial > 0)
    {        
        write(fd_serial, buff, 1);
    }    
}

static void read_serial(unsigned char * buffer, unsigned length)
{
    unsigned counter, delay_time;
    if(fd_serial > 0)
    {   
        ioctl(fd_serial,IOCMD_UART_GETRDBUFLEN,&counter);
        if(counter)
        {
            delay_time = (unsigned)(((float)1024 / (BAUD_SET_NUM >> 3)) * 1000 + 1) * 2;
            AVTimeDly(delay_time);
            ioctl(fd_serial,IOCMD_UART_GETRDBUFLEN,&counter);
            //read(fd_serial, buffer, counter);
            read(fd_serial, buffer, length);
        }
    }
}

static unsigned read_serial_all(unsigned char * buffer)
{
    unsigned counter;
    if(fd_serial > 0)
    {   
        ioctl(fd_serial,IOCMD_UART_GETRDBUFLEN,&counter);
        if(counter)
        {            
            ioctl(fd_serial,IOCMD_UART_GETRDBUFLEN,&counter);
            read(fd_serial, buffer, counter);
            return 0;
        }
    }
    
    return 1;
}

static void read_serial_byte(unsigned char *buff)
{
    unsigned counter;
    if(fd_serial > 0)
    {   
        ioctl(fd_serial,IOCMD_UART_GETRDBUFLEN,&counter);
        if(counter)
        {
            AVTimeDly(1);
            read(fd_serial, buff, 1);
            printf("0x%x\n", *buff); 
        }
    }
}

#define BUFF_SIZE 1024
unsigned Test_S_Flag = 0;
unsigned char r_buff[BUFF_SIZE];
unsigned char r_buff_all[128*BUFF_SIZE];
//unsigned char w_buff[BUFF_SIZE];
unsigned char tmp_buff = 0xAA;

static void InitTest(void)
{
    unsigned buff_len = BUFF_SIZE;
    if(fd_serial > 0)
    {
        //memset(w_buff, 'X', BUFF_SIZE);
        ioctl(fd_serial,IOCMD_UART_BUF,&buff_len);
        AVTimeDly(100);
    }
}

static void SerialTask(void *arg)
{
 /*   
    if(Test_S_Flag)
    {
        Test_S_Flag = 0;
        memset(r_buff, 0, BUFF_SIZE);
        //write_serial(w_buff, BUFF_SIZE);
        read_serial(r_buff, BUFF_SIZE);
    }
    
   */ 
//memset(r_buff, 0, BUFF_SIZE);
//read_serial(r_buff, BUFF_SIZE);
            //read_serial_byte(&tmp_buff);
            //write_serial(&tmp_buff,1);
            //read_serial(&tmp_buff, 1);
            //printf("0x%x\n",tmp_buff); 
    //write_serial_byte(&tmp_buff); 
    memset(r_buff_all,0,128*BUFF_SIZE);
    memset(r_buff, 0, BUFF_SIZE);
    printf("serial ready\n");
    
    while(1)
    {        
        if(!read_serial_all(r_buff))
        {
            strcat(r_buff_all,(const char *)r_buff);
            memset(r_buff, 0, BUFF_SIZE);
        }
        AVTimeDly(50);
    }          
    
    //AVTimeDly(1000);
}

static void InitSerialBase(void)
{
    *(volatile unsigned *)0x012000b8 &= ~((0xF<<9) | (0xF<<14));
    *(volatile unsigned *)0x012000b8 |= 1<<15;   //switch IOMUX0(RX) to UART MODE
    *(volatile unsigned *)0x012000b8 |= 1<<10;   //switch IOMUX1(TX) to UART MODE
    UART_DEV_PARA_T uart_para_cfg;
    fd_serial = open("/dev/uart",O_RDWR);
    uart_para_cfg.baud_rate = BAUD_SET_MARCO_P(BAUD_SET_NUM);//BAUD_RATE_115200;//BAUD_RATE_9600;
    uart_para_cfg.data_bit = DATA_BITS_8;
    uart_para_cfg.parity = PARITY_NO_CHECK;
    uart_para_cfg.stop_bit = STOP_BITS_1;
    if(fd_serial > 0)
    {   
        ioctl(fd_serial,IOCMD_UART_PARAM,&uart_para_cfg);
        AVTimeDly(100);
    }
}

void InitSerial(void)
{
    InitSerialBase();
    InitTest();
    if(fd_serial > 0)
        AVTaskCreate(SerialTask, 0, &TaskStartStkSerialTask[TASK_STK_SIZE - 1],16, &serial_task_id);
}