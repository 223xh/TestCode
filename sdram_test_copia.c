#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))
/***Copia 132M***/
#define DEFAULT_CLK 0x0958
/*****************/

#define TEST_SIZE 2*1024*1024
#define TEST_DATA_A 0x0000ffff
#define TEST_DATA_B 0x5555aaaa

unsigned *test_buff = 0;

static void my_set_freq(unsigned pll, unsigned div)
{
    unsigned tmp;
    
    tmp = rd(0x1100568);
    tmp &= ~(0x7fff);
    tmp |= (0x7fff & pll);
    wr(0x1100568, tmp);
    
    tmp = rd(0x1100574);
    tmp &= ~0xf;
    tmp |= (div & 0xf);
    wr(0x1100574, tmp);
}

static unsigned my_get_freq(void)
{
    unsigned m, n, div, od, clk;
    
    m = rd(0x1100568) & 0xff;
    n = ((rd(0x1100568)>>8) & 0x1f);
    od = ((rd(0x1100568)>>13) & 0x3);    
    div = rd(0x1100574) & 0xf;
    
    clk = 27 * m / n / (div + 1) / (unsigned)pow(2.0, (double)od);
    
    return clk;
}

static unsigned my_change_freq(unsigned clk)
{
    unsigned cur_clk;
    int step, t_step, c_step, i;
    unsigned tmp;
    
    cur_clk = my_get_freq();
    
    if((clk % 3) || (clk < 132) || (clk > 216))
    {
        printf("Invalid clock!\n");
        return 1;
    }
    
    step = clk>cur_clk?2:-2;
    
    i = 0;
    t_step = clk*2/3 - cur_clk*2/3;
    c_step = cur_clk*2/3 - 0x58;
    
    while(i != t_step)
    {
        i += step;
        tmp = DEFAULT_CLK + c_step + i;
        my_set_freq(tmp, 1);        
    }
    
    printf("Set clock to %uMhz done.\n",my_get_freq());
    
    return 0;
}

static void sdram_write(unsigned m_val)
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

static void sdram_read(unsigned m_val)
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

void sdram_test(void)
{
    unsigned clk_num;
    unsigned rand_num;
    
    my_set_freq(DEFAULT_CLK, 1);
    
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
        printf("Plz input clock(132-216,which is integral number of 3):\n");
        scanf("%d", &clk_num);
        
        if(my_change_freq(clk_num))
            continue;
        
        sdram_write(TEST_DATA_A);
        sdram_read(TEST_DATA_A);
        
        sdram_write(TEST_DATA_B);
        sdram_read(TEST_DATA_B);
        
        srand(AVTimeGet());
        rand_num = (unsigned)rand();
        sdram_write(rand_num);
        sdram_read(rand_num);
    }
}

