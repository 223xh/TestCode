#include <includes.h>
#include "ioapi.h"
#include "chipsupport/chipsupport.h"
#include <drivers/char/tty/tty_inc.h>
#include <drivers/usb/serial/usb_serial_inc.h>

#define tty_malloc AVMem_malloc
#define tty_free AVMem_free
#define TTY__TEST_DEVICE_TASK_PRI      29
#define TTY_TEST_STK_SIZE     (2*1024)
#define READ_THRESHOLD 10
#define TTY_RECEVIED_DATA_BUF_SZIE 0x1000

    #define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
    OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
    unsigned short my_task_id = 66;

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

static struct tty_device * dev;

static void tty_dev_close(struct tty_device *dev)
{
    close(dev->fd);
    dev->open_count -= 1;
}
static int tty_dev_open(struct tty_device *dev, char *name, int mode)
{
    dev->fd = open(name, mode);
    if(dev->fd < 0)
    {
        printf("tty_open: error when open the %s device!\n", name);
        return -1;
    }
    else
        printf("tty_open: ok.\n");
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

    static void MyTask(void *arg)
    {
    struct timios_data *termios;

    dev = tty_devcie_alloc();
    if(!dev) 
        return;
    termios = dev->termios;

    dev->fd = -1;
    AVTimeDly(5000);
    //tty_dev_open(dev, USB_SERIAL0_NAME, O_RDWR);
    while(dev->fd < 0)
    {
        
        tty_dev_open(dev, USB_SERIAL0_NAME, O_RDWR);
        AVTimeDly(2000);
    }
    tty_dev_close(dev);
    //tty_device_free(dev);
        //AVTimeDly(1000);
        tpdbg_uart_init();
        AVTaskDel(my_task_id);
    }
    
    void Init_Test_TTY()
    {
        AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);
    }