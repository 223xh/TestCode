#ifdef SDRAM_TEST
    OS_CPU_SR cpu_sr;
    #define PrintfStr(str)  OS_ENTER_CRITICAL();printf(str);OS_EXIT_CRITICAL();
    #define PrintfVar(var)  OS_ENTER_CRITICAL();printf("%s",var);OS_EXIT_CRITICAL();
    #define PrintfINT(num)  OS_ENTER_CRITICAL();printf("%d\n",num);OS_EXIT_CRITICAL();

    #define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
    OS_STK TaskStartStkSdramTest[TASK_STK_SIZE];
    unsigned short sdram_test_task_id = 66;//0;
    
    void SdramTestTask(void *);
    
    #define MEMSIZE (1024 * 2048)           //2M
    #define MEMLENGTH  (INT32U)(MEMSIZE>>4)    //2M/16 * 4 =512K
    
    INT32U sFlag = 0;//0xFF;
    INT32U numOff = 0;  //offset MAX=15
    INT32U *numMem = NULL;    
    INT32U numWR = 0;
    INT32U numErr = 0;
#endif

#ifdef SDRAM_TEST
    AVTaskCreate(SdramTestTask, NULL, &TaskStartStkSdramTest[TASK_STK_SIZE - 1],20, &sdram_test_task_id);
#endif

#ifdef SDRAM_TEST
    void SdramTestTask(void *arg)
    {
        if(sFlag == 0)
        {
            PrintfStr("Start to test sdram.\n");
            numMem = (INT32U *)AVMem_malloc(sizeof(INT32U)*MEMLENGTH);
            if(numMem == 0)
            {
                sFlag = 0xEE;
                PrintfStr("AVMem_malloc failed.\n");
                AVMem_free(numMem);
                numMem = NULL;
                AVTaskDel(sdram_test_task_id);
            }
            else
            {
                sFlag += 1;
                PrintfStr("AVMem_malloc successful.\n");
                memset(numMem, 0, sizeof(INT32U)*(MEMLENGTH));
            }
        }
        else if(sFlag == 1)
        {
            if(numWR < MEMLENGTH)
            {
                if(numWR == 0)
                {
                    PrintfStr("Start to write sdram.\n");
                }
                numMem[numWR] = (INT32U)((numWR + 1) << numOff);
                numWR += 1;
            }
            else
            {
                sFlag += 1;
                PrintfStr("Write sdram finished.\n");
                numWR = 0;
            }
        }
        else if(sFlag == 2)
        {
            if(numWR < MEMLENGTH)
            {
                if(numWR == 0)
                {
                    PrintfStr("Start to read sdram.\n");
                }
                if(numMem[numWR] == (INT32U)((numWR + 1) << numOff))
                {
                    //PrintfINT(numMem[numWR]);                    
                }
                else
                {
                    /*
                    PrintfStr("Below number is different from original number --> ");
                    PrintfINT((INT32U)((numWR + 1) << numOff));
                    PrintfINT(numMem[numWR]);
                    */
                    numErr += 1;
                }                
                numWR += 1;
            }
            else
            {
                sFlag = 0xFF;
                PrintfStr("Read sdram finished.\n");
                numWR = 0;
                
                PrintfStr("Total error number = ");
                PrintfINT(numErr);
                
                PrintfStr("\n");
                
                AVMem_free(numMem);
                numMem = NULL;
                numErr = 0;
            }
        }
        //AVTaskDel(sdram_test_task_id);
    }
#endif