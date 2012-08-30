#include <includes.h>
#include <ioapi.h>
#include <displaydev.h>

#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

#define BUFFER_SIZE 800*600*3

unsigned char* rgb_buff = 0;
video_screen_hardcopy_req_t m_sc;

static void GetRGB()
{
    int vfd = -1;
    unsigned int len = 0;
    
    
    vfd = open("/dev/video", O_RDWR);
    if(vfd)
    {
        len = BUFFER_SIZE;
        m_sc.flag = VIDEO_SCFLAG_RESTART;
        m_sc.data = rgb_buff;
        m_sc.len = &len;
        
        ioctl(vfd, VIDEOIO_SCREEN_HARDCOPY, &m_sc);
        close(vfd);
    }
}

static unsigned char m_flag = 0;

static void MyTask(void *arg)
{    
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