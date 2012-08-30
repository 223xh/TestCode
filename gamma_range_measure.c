#ifdef GAMMA_RANGE_MEASURE
    #define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
    OS_STK TaskStartStkGRM[TASK_STK_SIZE];
    unsigned short grm_task_id = 66;//0;
    
    void GRMTask(void *);
    
    #define MASK_R 0xc00
    #define MASK_G 0xa00
    #define MASK_B 0x900
    INT32U gamma_threshold = 0;
    INT32U gamma_flag = 0;    
    void program_gamma(int);
#endif

#ifdef GAMMA_RANGE_MEASURE
    AVTaskCreate(GRMTask, 0, &TaskStartStkGRM[TASK_STK_SIZE - 1],20, &grm_task_id);
#endif

#ifdef GAMMA_RANGE_MEASURE
    void GRMTask(void *arg)
    {
        if(gamma_flag)
        {
            gamma_flag = 0;
            
            program_gamma(MASK_R);
		        program_gamma(MASK_G);
		        program_gamma(MASK_B);
        }
         //AVTaskDel(grm_task_id);
    }
    
    void program_gamma(int color)
    {
        while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
        *(volatile unsigned *)0x01101208 = color;
        
        for (int i=0;i<256;i++) {
            while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
            if(i < gamma_threshold)
                *(volatile unsigned *)0x01101204 = 0;
            else if(i > gamma_threshold)
                *(volatile unsigned *)0x01101204 = 1023;
            else if(i = gamma_threshold)
                *(volatile unsigned *)0x01101204 = gamma_threshold << 2;
        }
    
        while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
        *(volatile unsigned *)0x01101208 = (color | 0x23);
    }
#endif
