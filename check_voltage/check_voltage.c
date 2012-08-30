    #define  TASK_STK_SIZE              1024       /* Size of each task's stacks (# of WORDs)            */
    OS_STK TaskStartStkMyTask[TASK_STK_SIZE];
    unsigned short my_task_id = 66;
       
    #define MAX_PWM         260
    #define RES_COEFF       ((float)3/5)
    #define VOLTAGE_DROP    0
    #define TURN_NUM        4
    #define V_CHK_COND      (*(volatile unsigned *)0x12000a8 & (1<<5))
     
    void MyTask(void *arg)
    {
        unsigned char n = TURN_NUM;
        unsigned char last_drct = 1;
        float v_val;
        int high_val = MAX_PWM/2;
        int low_val = MAX_PWM/2;
        float last_val = (3.3*((float)high_val/MAX_PWM));
        
        set_lcd_pwm0(low_val, high_val);
        AVTimeDly(10);
        printf("start checking voltage...\n");
        
        while(1)
        {
            if(V_CHK_COND)
            {
                high_val = (high_val+1)>MAX_PWM?MAX_PWM:(high_val+1);
                low_val = (low_val-1)>0?(low_val-1):0;
            }
            else
            {
                high_val = (high_val-1)>0?(high_val-1):0;
                low_val = (low_val+1)>MAX_PWM?MAX_PWM:(low_val+1);
            }
            set_lcd_pwm0(low_val, high_val);
            
            if(last_drct == !V_CHK_COND)
            {
                n = TURN_NUM;
            }
            else
            {
                last_drct = !V_CHK_COND;
                --n;
                last_val = (3.3*((float)high_val/MAX_PWM));
            }
            
            if(!n)
            {
                n = TURN_NUM;
                v_val = (3.3*((float)high_val/MAX_PWM));
                v_val = (((v_val + last_val) / 2) / (RES_COEFF)) + VOLTAGE_DROP;
                printf("voltage is %.3fV\n",v_val);
                AVTimeDly(1000);
            }
            else
                AVTimeDly(10);
        }        
    }


AVTaskCreate(MyTask, 0, &TaskStartStkMyTask[TASK_STK_SIZE - 1],16, &my_task_id);