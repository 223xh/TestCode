/*****************************************************************
**                                                                 **
**  Copyright (C) 2004 Amlogic,Inc.                                **
**  All rights reserved                                            **
**        Filename : tp_uart.c /Project:AVOS                      ** 
**        Revision : 1.0                                           **
**                                                                 **
*****************************************************************/

#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include <drivers/char/tty/tty_inc.h>
#include <drivers/usb/serial/usb_serial_inc.h>

#include "irtc.h"

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

#define MAX_READ_LENGTH 1024

#define BUF_SIZE    16
#define tty_malloc AVMem_malloc
#define tty_free AVMem_free
#define TTY__TEST_DEVICE_TASK_PRI      29
#define TTY_TEST_STK_SIZE     (2*1024)
#define READ_THRESHOLD 10
#define TTY_RECEVIED_DATA_BUF_SZIE 0x1000

struct tty_device {
    int fd;
    int  open_count;
    OS_STK *TTY_DEVICE_Stk;
    INT16U     tty_task_id;
    struct tty_event        *read_event;
    u8                 is_thread_creat;
    u8                 is_back_display; 
    struct timios_data *termios;
    char * recevied_data_buf;
    int data_thretshold;
};

struct tty_device * dev;

static void tty_dev_close(struct tty_device *dev)
{
    close(dev->fd);
    dev->open_count -= 1;
}
static int tty_dev_open(struct tty_device *dev, char *name, int mode)
{
    dev->fd = open(name, mode);
    if(dev->fd < 0) {
        printf("tty_open: error when open the %s device!\n", name);
        return -1;
    }
    else
        printf("tty_open: ok.running...\n");
    return dev->fd;    
}

static struct tty_device *tty_devcie_alloc()
{
    struct tty_device *dev;
    dev = tty_malloc(sizeof(struct tty_device));
    if(!dev)
        return 0;
    memset(dev, 0, sizeof(struct tty_device));
    dev->termios = tty_malloc(sizeof(struct timios_data));
    if(!dev->termios) {
        tty_free(dev);
        return 0;
    }
    dev->recevied_data_buf = tty_malloc(TTY_RECEVIED_DATA_BUF_SZIE);
    if(!dev->recevied_data_buf) {
        tty_free(dev->termios);
        tty_free(dev);
        return 0;
    }
    dev->read_event = tty_malloc(sizeof(struct tty_event));
    if(!dev->read_event) {
        tty_free(dev->termios);
        tty_free(dev);
        tty_free(dev->recevied_data_buf);
    }
    return dev;
}

static void tty_device_free(struct tty_device *dev)
{
    if(dev->termios) {
        tty_free(dev->termios);
        dev->termios = NULL;
    }
    if(dev){
        tty_free(dev);
        dev = NULL;
    }    
    tty_free(dev->read_event);
}

static int tty_test_sleep_thread(struct tty_device *dev)
{
    u8 ret;

    AVSemPend(dev->read_event->event, 0, &ret);
    if (ret != OS_NO_ERR)
        return -1;
    
    return 0;
}

static unsigned char split_cmd(struct tty_device *dev, char *cach, char param[][BUF_SIZE])
{
    unsigned char i, j, k;
    char s[45];
    
    j = k = 0;
    memset(s,0,45);
    
    for(i = 0; i < strlen(cach); i++)
    {
        if(cach[i] == 32)
        {
            if(j == 0)
                continue;
            j = 0;
            if(k < 4)
                ++k;
            else
                goto ERROR;
        }
        else
        {
            if(k == 0 && j >= 16)
                goto OVER1;
                
            if(k > 0 && j >= 10)
                goto OVER2;
                
            param[k][j] = cach[i];
            ++j;
        }
    }
    
    return 0;

ERROR:
    write(dev->fd,"Too many parameters!\xD\xA",22);
    return 4;
OVER1:
    sprintf(s,"Length of parameter %d must be less than 32!\xD\xA",k);
    write(dev->fd,s,45);
    return 1;
OVER2:
    sprintf(s,"Length of parameter %d must be less than 10!\xD\xA",k);
    write(dev->fd,s,45);
    return 3;
}

static void process_cmd(struct tty_device *dev, char param[][BUF_SIZE])
{
    unsigned mem_addr, mem_len, mem_val, mem_mask,mem_val_cach, i;
    unsigned char mem_offset;
    unsigned char cmd_flag = 0;
    char strPrt[BUF_SIZE * 2];
    char *strErr;
    
    memset(strPrt,0,BUF_SIZE * 2);
    
    if(strcmp(param[0],"clear") == 0)
    {
        write(dev->fd,"\xC\xC",2);
    }
    else if(strcmp(param[0],"getmem") == 0)
    {
        if(strlen(param[1]) == 0)
            goto ERROR1;
            
        if((param[1][0] == 48 && (param[1][1] == 120 || param[1][1] == 88)) == 0)
            goto ERROR2;
            
        mem_addr = strtol(param[1], &strErr, 0);
        if(strlen(strErr))
            goto ERROR2;
            
        if(strlen(param[2]))
        {
            mem_len = strtol(param[2], &strErr, 0);
            if(strlen(strErr))
                goto ERROR3;
                
            if(mem_len > MAX_READ_LENGTH)
                goto ERROR4;
            for(i=0;i<mem_len>>2;i++)
            {
                memset(strPrt,0,BUF_SIZE * 2);
                sprintf(strPrt,"0x%08x: 0x%08x\xD\xA", mem_addr+(i<<2), rd(mem_addr+(i<<2)));
                write(dev->fd, strPrt, strlen(strPrt));
            }
        }
        else
        {
            sprintf(strPrt,"0x%08x: 0x%08x\xD\xA", mem_addr, rd(mem_addr));
            write(dev->fd, strPrt, strlen(strPrt));
        }
    }
    else if(strcmp(param[0],"setmem") == 0)
    {
        cmd_flag = 1;
        
        if(strlen(param[1]) == 0)
            goto ERROR1;
            
        if(!(param[1][0] == 48 && (param[1][1] == 120 || param[1][1] == 88)))
            goto ERROR2;
            
        mem_addr = strtol(param[1], &strErr, 0);
        if(strlen(strErr))
            goto ERROR2;
        
        if(strlen(param[2]) == 0)
            goto ERROR3;
        
        mem_val = strtol(param[2], &strErr, 0);
        if(strlen(strErr))
            goto ERROR3;
            
        if(strlen(param[3]))
        {
            mem_mask = strtol(param[3], &strErr, 0);
            if(strlen(strErr))
                goto ERROR5;
            
            mem_val_cach = rd(mem_addr);
                
            if(strlen(param[4]))
            {
                mem_offset = strtol(param[4], &strErr, 0);
                if(strlen(strErr))
                    goto ERROR6;
                
                if(mem_offset > 31)
                    goto ERROR7;
                    
                mem_val_cach &= ~(mem_mask << mem_offset);
                mem_val_cach |= (mem_val & mem_mask) << mem_offset;
            }
            else
            {
                mem_val_cach &= ~mem_mask;
                mem_val_cach |= mem_val & mem_mask;
            }
        }
        else
        {
            mem_val_cach = mem_val;
        }
        
        wr(mem_addr, mem_val_cach);
        sprintf(strPrt,"0x%08x: 0x%08x\xD\xA", mem_addr, rd(mem_addr));
        write(dev->fd, strPrt, strlen(strPrt));
    }
    else
    {
        write(dev->fd,"Unknown command\xD\xA",18);
    }
    return;

ERROR7:
    write(dev->fd,"Offset must be less than 32!\xD\xA",30);
    return;
ERROR6:
    write(dev->fd,"Invalid Parameter 4!\xD\xA",22);
    goto ERROR1;
ERROR5:
    write(dev->fd,"Invalid Parameter 3!\xD\xA",22);
    goto ERROR1;
ERROR4:
    write(dev->fd,"Over MAX memory length(1k)!\xD\xA",29);
    return;
ERROR3:
    write(dev->fd,"Invalid Parameter 2!\xD\xA",22);
    goto ERROR1;
ERROR2:
    write(dev->fd,"Invalid Parameter 1!\xD\xA",22);
ERROR1:
    if(cmd_flag)
        write(dev->fd,"setmem memory(0x********) value [mask] [offset]\xD\xA",49);
    else
        write(dev->fd,"getmem memory(0x********) [length(bytes)]\xD\xA",43);
    return;
}

#define STR_SIZE 100
#define HI_CHK_VAL 2000000000
#define HI_UNIT ((double)4294967.296)

#define C_TIMER_MUX (*(volatile unsigned *)0x01000530)
#define C_TIMER_E (*(volatile unsigned *)0x01000544)

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

static void tpdbg_uart_task(void *arg)
{
    struct tty_device *dev = (struct tty_device *)arg;
    struct tty_data_avail_cmd cmd;
    unsigned buff_len=0;
    unsigned char i, rd_len=0;
    unsigned char buff[BUF_SIZE];
    char cmd_cach[BUF_SIZE * 6];    
    char cmd_param[5][BUF_SIZE];
    unsigned char rw_flag = 1;
    unsigned char rw_pointer = 0;
    
    memset(cmd_cach,0,79);
    memset(cmd_param,0,BUF_SIZE * 5);
    
    char str[STR_SIZE];
    char r_buff;
    unsigned char r_flag = 3;
    unsigned char sw_flag = 3;
    unsigned char bypass_flag = 3;
    unsigned char z_flag = 3;
    
    unsigned rtc_base, rtc_val, av_val, hi_val = 0;
    unsigned char av_flag = 0;
    OS_CPU_SR  cpu_sr;
    
    //memset(str,0,STR_SIZE);
    
    static_reg_write(0x4);
    irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
    irtc_write(RTC_SEC_ADJUST_ADDR, 0);
    (*RTC_REG3) &= ~(1 << 17);
    (*RTC_REG2) = 0;
    (*RTC_REG3) |= (1 << 17);// Enable count always
    
    OS_ENTER_CRITICAL();
    //OS_EXIT_CRITICAL();
    rtc_base = irtc_read(RTC_COUNTER_ADDR);
    OS_EXIT_CRITICAL();
    AVTimeSet(0);
    AVTimeDly(2000);

    while(1)
    {
        av_val = AVTimeGet();
        rtc_val = (unsigned)(rtc_base + HI_UNIT*hi_val + ((double)av_val/1000));
        OS_ENTER_CRITICAL();
        irtc_write(RTC_COUNTER_ADDR, rtc_val);
        OS_EXIT_CRITICAL();
        
        if(av_val > HI_CHK_VAL)
        {
            if(!av_flag)
                av_flag = 1;
        }
        else
        {
            if(av_flag)
            {
                av_flag = 0;
                ++hi_val;
            }
        }
        
        AVTimeDly(20);
        //delay_ms(20);
        memset(str,0,STR_SIZE);
        OS_ENTER_CRITICAL();
        sprintf(str,"%u[0x%x],T=%u,OSC=%u\xD\xA",irtc_read(RTC_COUNTER_ADDR),(*(RTC_REG0)>>24) & 0xff,AVTimeGet(),(*RTC_REG2));
        OS_EXIT_CRITICAL();
        write(dev->fd,str,strlen(str));
        
        cmd.tty_buf_index = TTY_READ_BUF_INDEX;
        ioctl(dev->fd,TTY_IOCTL_GET_DATA_LEN,&cmd);
        buff_len = cmd.data_avail;
        if(!buff_len)
        {
            AVTimeDly(869);
            //delay_ms(869);
            continue;
        }
        
CHECK_READ_BUFFER:
                        
        read(dev->fd,&r_buff,1);
        if(r_buff == 120)//'x'
            --r_flag;
        else
            r_flag = 3;
            
        if(r_buff == 99)//'c'
            --bypass_flag;
        else
            bypass_flag = 3;
            
        if(r_buff == 48)//'0'
            --z_flag;
        else
            z_flag = 3;
            
        if(buff_len>1)
        {
            --buff_len;
            goto CHECK_READ_BUFFER;
        }
            
        if(!r_flag)
        {
            r_flag = 3;
            if(sw_flag == 1)
                static_reg_write(0x0);
            else if(sw_flag == 2)
                static_reg_write(0x4);
            else if(sw_flag == 3)
                static_reg_write(0xe);
            else
                static_reg_write(0x4);
                
            ++sw_flag;
            if(sw_flag>3)
            {
                static_reg_write(0x0);
                static_reg_write(0xe);
                sw_flag = 1;
            }
                
            memset(str,0,STR_SIZE);
            sprintf(str,"static reg = [0x%x]\xD\xA",(*(RTC_REG0)>>24) & 0xff);
            write(dev->fd,str,strlen(str));
        }
        
        if(!bypass_flag)
        {
            bypass_flag = 3;
            if(*(RTC_REG0) & (1<<3))
                *(RTC_REG0) &= ~(1<<3);
            else
                *(RTC_REG0) |= (1<<3);
                            
            memset(str,0,STR_SIZE);
            
            if(*(RTC_REG0) & (1<<3))
                sprintf(str,"Enable bypass.\xD\xA");
            else
                sprintf(str,"Disable bypass.\xD\xA");
            write(dev->fd,str,strlen(str));
        }
        
        if(!z_flag)
        {
            z_flag = 3;
            
            AVTimeDly(20);
            //delay_ms(20);
            OS_ENTER_CRITICAL();
            irtc_write(RTC_COUNTER_ADDR, 0);
            OS_EXIT_CRITICAL();
            (*RTC_REG3) &= ~(1 << 17);
            (*RTC_REG2) = 0;
            (*RTC_REG3) |= (1 << 17);
            memset(str,0,STR_SIZE);
            sprintf(str,"Set RTC_COUNTER_ADDR & OSC to 0 done.\xD\xA");            
            write(dev->fd,str,strlen(str));
        }
        
        
        AVTimeDly(890);
        //delay_ms(870);
        /*
        tty_test_sleep_thread(dev);
        
        if(dev->fd < 0)
            continue;
        while(1)
        {
            if(rw_flag == 2)
            {
                rw_flag = 1;
                if(split_cmd(dev, cmd_cach, cmd_param) == 0)
                {
                    if(strlen(cmd_param[0]) > 0)
                    {
                        process_cmd(dev, cmd_param);                    
                    }
                }
                rw_pointer = 0;
                memset(cmd_cach,0,79);
                memset(cmd_param,0,BUF_SIZE * 5);
            }
            else
            {
                cmd.tty_buf_index = TTY_READ_BUF_INDEX;
                ioctl(dev->fd,TTY_IOCTL_GET_DATA_LEN,&cmd);
                buff_len = cmd.data_avail;
                if(!buff_len)
                    break;
                    
                memset(buff,0,BUF_SIZE);
                if(buff_len<BUF_SIZE)
                {
                    read(dev->fd,&buff,buff_len);
                    rd_len = buff_len;
                }
                else
                {
                    read(dev->fd,&buff,BUF_SIZE);
                    rd_len = BUF_SIZE;
                }
                
                for(i = 0; i < rd_len; i++)
                {
                    if((buff[i] >= 48 && buff[i] <= 57) || (buff[i] >= 65 && buff[i] <= 90) || (buff[i] >= 97 && buff[i] <= 122) || buff[i] == 13 || buff[i] == 32 || buff[i] == 8)
                    {                        
                        if(buff[i] == 13)
                        {
                            rw_flag = 2;
                            write(dev->fd,"\xD\xA",2);
                            break;
                        }
                        else if(buff[i] == 8)
                        {
                            if(rw_pointer)
                            {
                                --rw_pointer;
                                cmd_cach[rw_pointer] = 0;                                
                                write(dev->fd,"\x8\x20\x8",3);
                            }
                        }                        
                        else
                        {
                            if(rw_pointer < 79)
                            {
                                ++rw_pointer;
                                cmd_cach[rw_pointer - 1] = buff[i];
                                write(dev->fd,(char *)&buff[i],1);
                            }
                        }
                    }
                }
            }
        }
        */
    }
    return;
}
    
int tpdbg_uart_init(void)
{
    int ret;
    u8 flag;
    struct timios_data *termios;

    dev = tty_devcie_alloc();
    if(!dev) 
        return -1;
    termios = dev->termios;

    tty_dev_open(dev, USB_SERIAL0_NAME, O_RDWR);    
    if(dev->fd < 0) {
        printf("tty_init: error when open the %s file\n", USB_SERIAL0_NAME);
        return -1;
    }

    ret = ioctl(dev->fd, TTY_IOCTL_GET_TERMIOS, termios);    
    if(ret < 0) {
        printf("tty_init: get timios error!\n");
        return -1;
    }

    termios->baudrate = 115200;//460800;
    termios->stop_bit = 1;
    termios->check = 0;
    termios->databits = 8;    
    ret = ioctl(dev->fd, TTY_IOCTL_SET_TERMIOS, termios);
    if(ret < 0)
    {
        printf("tty_init: set timios error!\n");
        goto cleanup;
    }    

    ret = ioctl(dev->fd, TTY_IOCTL_CLEAR_BACK_DISPLAY,0);
    if(ret < 0)
        goto cleanup;
    

    ret = ioctl(dev->fd, TTY_IOCTL_GET_DEV_EVENT, dev->read_event);
    dev->read_event->event = AVSemCreate(0);
    
    if (!dev->read_event->event)
        goto cleanup;
    dev->read_event->tty_call_back = 0;
    dev->read_event->args = 0;

    dev->TTY_DEVICE_Stk = (OS_STK *)tty_malloc(TTY_TEST_STK_SIZE * sizeof(OS_STK));
    if(!dev->TTY_DEVICE_Stk)
        goto cleanup;
    ret = AVTaskCreate(tpdbg_uart_task, dev, dev->TTY_DEVICE_Stk + TTY_TEST_STK_SIZE - 1, TTY__TEST_DEVICE_TASK_PRI, &dev->tty_task_id);
    if (ret != OS_NO_ERR) {
        goto cleanup;
    }
    dev->is_thread_creat = 1;
    dev->data_thretshold = READ_THRESHOLD;
    
    
    ret = ioctl(dev->fd, TTY_IOCTL_SET_DEV_EVENT, dev->read_event);
    if(ret < 0)
        printf("tty_init: set timios error!\n");
    return 0;
    
cleanup:
    if(dev->read_event->event)
        AVSemDel(dev->read_event->event, OS_DEL_ALWAYS, &flag);
    if(dev)
        tty_device_free(dev);
    return -1;   
}
