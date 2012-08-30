#if 1
#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

#define TEST_SIZE 2*1024*1024
#define TEST_DATA_A 0x0000ffff
#define TEST_DATA_B 0x5555aaaa
#define TEST_DATA_N 0xa5a5a5a5
#define TEST_TIMES 160//480

unsigned *test_buff = 0;
unsigned *test_buff_a = 0;
unsigned *test_buff_b = 0;
unsigned *test_buff_c = 0;

//{m,n}
unsigned arr_clk[18][2]=
{
    //180M, 184M,   192M,   200M,   204M,   208M,
    {15, 2},{23, 3},{16, 2},{25, 3},{17, 2},{26, 3},
    //216M, 224M,   228M,   232M,   240M,   248M,   
    {18, 2},{28, 3},{19, 2},{29, 3},{20, 2},{31, 3},
    //252M, 256M,   264M,   272M,   276M,   280M
    {21, 2},{32, 3},{22, 2},{34, 3},{23, 2},{35, 3}
};

unsigned Get_DDR_Delay()
{
    unsigned tmp;
    tmp = *(volatile unsigned*)0xc1104e04;
    
    return ((tmp>>8)&7);
}

static void Plus_DDR_CLK()
{
    unsigned tmp = 10000;
    
    rd(0xc1104e08) += 1;
    
    while(--tmp);
    
    while((rd(0xc1104e40) & (1<<15)) == 0);
}

void My_Set_DDR_CLK(unsigned m, unsigned n)
{
    unsigned tmp;
    
    tmp = rd(0xc1104e08);
    tmp &= ~(0x3fff);
    tmp |= (((0x1f & n)<<9) | (0x1ff & m));
    
    wr(0xc1104e08, tmp);
    
    tmp = 10000;
    while(--tmp);
    
    while((rd(0xc1104e40) & (1<<15)) == 0);
}

float My_Get_DDR_CLK()
{
    unsigned m, n;
    float clk;
    
    m = rd(0xc1104e08) & 0x1ff;
    n = ((rd(0xc1104e08)>>9) & 0x1f);    
    
    clk = (float)24 * m / n;
    
    return clk;
}

static void ddr_write(unsigned *buff, unsigned m_val, unsigned flag)
{
    unsigned *p;
    unsigned i;
    
    if(!flag)
        printf("Start testing 0x%08x.\n",m_val);
    
    p = buff;
    
    for(i=0;i<TEST_SIZE/4;i++)
    {
        *p = m_val;
        ++p;
    }    
}

static void ddr_read(unsigned *buff, unsigned m_val, unsigned flag)
{
    unsigned *p;
    unsigned e_num = 0;
    unsigned e_buff = 0;
    unsigned i;
    
    p = buff;
    
    for(i=0;i<TEST_SIZE/4;i++)
    {
        if(*p != m_val)
        {            
            if(e_buff != *p)
            {
                if(e_num)
                {
                    printf("Error buffer is 0x%x, error number %u, opposite addr is 0x%08x\n",e_buff, e_num, i*4);
                    e_num = 0;
                }
                e_buff = *p;
            }
            
            ++e_num;
        }  
        else
        {
            if(e_num)
            {
                printf("Error buffer is 0x%x, error number %u, opposite addr is 0x%08x, flag[%u]\n",e_buff, e_num, i*4, flag);
                e_num = 0;
                e_buff = 0;
            }
        }      
        
        ++p;
    }
    
    if(flag == TEST_TIMES-1)
        printf("Test 0x%08x finished.\n", m_val);
}

static void ddr_read_silent()
{
    unsigned *p;
    unsigned t_buff;
    unsigned i = TEST_SIZE>>2;
    
    p = test_buff;

    do
    {
        t_buff = *p;
        ++p;
    }
    while(--i);
}

void ddr_test()
{
//    unsigned clk_num;
    unsigned rand_num;
    
    unsigned i, j;
    
//    unsigned s, e, i, n;
    
//    test_buff = (unsigned *)AVMem_malloc(TEST_SIZE);
//    if(test_buff)
//        printf("Malloc memory OK.\n");
//    else
//    {
//        printf("Malloc memory failed.Exit test.\n");
//        return;
//    }
    
    test_buff_a = (unsigned *)AVMem_malloc(TEST_SIZE);
    if(test_buff_a)
        printf("Malloc memory a OK.\n");
    else
    {
        printf("Malloc memory a failed.Exit test.\n");
        return;
    }
    
    test_buff_b = (unsigned *)AVMem_malloc(TEST_SIZE);
    if(test_buff_b)
        printf("Malloc memory b OK.\n");
    else
    {
        printf("Malloc memory b failed.Exit test.\n");
        return;
    }
    
    test_buff_c = (unsigned *)AVMem_malloc(TEST_SIZE);
    if(test_buff_c)
        printf("Malloc memory c OK.\n");
    else
    {
        printf("Malloc memory c failed.Exit test.\n");
        return;
    }
    
//    srand(AVTimeGet());
//    rand_num = (unsigned)rand();
//    AVTimeDly(rand_num&0xfff);
//    
//    printf("DDR CLK is %fMhz, Delay is %u\n", My_Get_DDR_CLK(), Get_DDR_Delay());
//    ddr_write(test_buff_a, TEST_DATA_A);
//    ddr_write(test_buff_b, TEST_DATA_B);
//    
//    srand(AVTimeGet());
//    rand_num = (unsigned)rand();
//    ddr_write(test_buff_c, rand_num);
    
    for(i=0;i<18;i++)
    {
        My_Set_DDR_CLK(arr_clk[i][0],arr_clk[i][1]);
        printf("DDR CLK is %fMhz, Delay is %u\n", My_Get_DDR_CLK(), Get_DDR_Delay());
        for(j=0;j<TEST_TIMES;j++)
        {
            memset(test_buff_a, 0, TEST_SIZE);
            ddr_write(test_buff_a, TEST_DATA_A, j);
            ddr_read(test_buff_a, TEST_DATA_A, j);
        }
        for(j=0;j<TEST_TIMES;j++)
        {
            memset(test_buff_b, 0, TEST_SIZE);
            ddr_write(test_buff_b, TEST_DATA_B, j);
            ddr_read(test_buff_b, TEST_DATA_B, j);
        }
        for(j=0;j<TEST_TIMES;j++)
        {
            memset(test_buff_c, 0, TEST_SIZE);
            srand(AVTimeGet());
            rand_num = (unsigned)rand();
            ddr_write(test_buff_c, rand_num, j);
            ddr_read(test_buff_c, rand_num, j);
        }
    }
    
    
//    s = AVTimeGet();
//    memset(test_buff,0xa5,TEST_SIZE);
//    e = AVTimeGet();
//    printf("memset used time=%ums,DDR CLK is %fMHz,Delay is %u\n",e-s,My_Get_DDR_CLK(),Get_DDR_Delay());
//    
//    i = 20;
//    s = AVTimeGet();
//    do
//    {
//        ddr_read_silent();
//    }
//    while(--i);
//    e = AVTimeGet();
//    printf("Read used time=%ums,DDR CLK is %fMHz,Delay is %u\n",e-s,My_Get_DDR_CLK(),Get_DDR_Delay());
//    
//    n = 5;
//    do
//    {
//        Plus_DDR_CLK();
//        i = 20;
//        s = AVTimeGet();
//        do
//        {
//            ddr_read_silent();
//        }
//        while(--i);
//        e = AVTimeGet();
//        printf("Read used time=%ums,DDR CLK is %fMHz,Delay is %u\n",e-s,My_Get_DDR_CLK(),Get_DDR_Delay());
//    }
//    while(--n);
    
    while(1)
    {
//        printf("Plz input CLK:\n");
//        scanf("%d", &clk_num);
//        my_set_freq(clk_num, 24);
//        printf("DDR CLK is %fMhz\n", my_get_freq());
//        
//        ddr_write(TEST_DATA_A);
//        ddr_read(TEST_DATA_A);
//        
//        ddr_write(TEST_DATA_B);
//        ddr_read(TEST_DATA_B);
//        
//        srand(AVTimeGet());
//        rand_num = (unsigned)rand();
//        ddr_write(rand_num);
//        ddr_read(rand_num);
    }
}
#endif
