#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

#define TEST_SIZE 16*1024*1024
#define TEST_DATA_A 0x0000ffff
#define TEST_DATA_B 0x5555aaaa

unsigned *test_buff = 0;

void my_set_freq(unsigned m, unsigned n)
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

static float my_get_freq(void)
{
    unsigned m, n;
    float clk;
    
    m = rd(0xc1104e08) & 0x1ff;
    n = ((rd(0xc1104e08)>>9) & 0x1f);    
    
    clk = (float)24 * m / n;
    
    return clk;
}

static void ddr_write(unsigned m_val)
{
    unsigned *p;
    unsigned i;
    
    printf("Start testing 0x%08x.\n",m_val);
    
    p = test_buff;
    
    for(i=0;i<TEST_SIZE/4;i++)
    {
        *p = m_val;
        ++p;
    }    
}

static void ddr_read(unsigned m_val)
{
    unsigned *p;
    unsigned e_num = 0;
    unsigned e_buff = 0;
    unsigned i;
    
    p = test_buff;
    
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
                printf("Error buffer is 0x%x, error number %u, opposite addr is 0x%08x\n",e_buff, e_num, i*4);
                e_num = 0;
                e_buff = 0;
            }
        }      
        
        ++p;
    }
    
    printf("Test 0x%08x finished.\n", m_val);
}

void ddr_test()
{
    unsigned clk_num;
    unsigned rand_num;
    
    test_buff = (unsigned *)AVMem_malloc(TEST_SIZE);
    if(test_buff)
        printf("Malloc memory OK.\n");
    else
    {
        printf("Malloc memory failed.Exit test.\n");
        return;
    }
    
    while(1)
    {
        printf("Plz input CLK:\n");
        scanf("%d", &clk_num);
        my_set_freq(clk_num, 24);
        printf("DDR CLK is %fMhz\n", my_get_freq());
        
        ddr_write(TEST_DATA_A);
        ddr_read(TEST_DATA_A);
        
        ddr_write(TEST_DATA_B);
        ddr_read(TEST_DATA_B);
        
        srand(AVTimeGet());
        rand_num = (unsigned)rand();
        ddr_write(rand_num);
        ddr_read(rand_num);
    }
}

