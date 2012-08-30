#define MASK_R 0xc00
#define MASK_G 0xa00
#define MASK_B 0x900

unsigned gamma_flag = 0;
unsigned gamma_r,gamma_g,gamma_b;
    
void MyTask(void *);

static void program_gamma_level(int color, unsigned short val)
{
	int i;
		
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = color;
                
    for (i=0;i<256;i++)
    {
       while (!(*(volatile unsigned *)0x01101200 & (0x1 << 4)));
       *(volatile unsigned *)0x01101204 = val;
        
    }
    
    while (!(*(volatile unsigned *)0x01101200 & (0x1 << 5)));
    *(volatile unsigned *)0x01101208 = (color | 0x23);
}

void MyTask(void *arg)
{
    if(gamma_flag)
    {
        gamma_flag = 0;
        program_gamma_level(MASK_R,gamma_r);
        program_gamma_level(MASK_G,gamma_g);
        program_gamma_level(MASK_B,gamma_b);
    }
    AVTimeDly(1000);
    //AVTaskDel(my_task_id);
}