#define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
static unsigned short my_task_id = 66;

static void MyTask(void *arg)
{
    
    AVTimeDly(100);
    //AVTaskDel(my_task_id);
}

AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],20, &my_task_id);