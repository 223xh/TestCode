#if 1
#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

#define TEST_SIZE 8*1024*1024

unsigned *test_buff = 0;

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

static void Set_DDR_Delay(unsigned delay)
{
    *(volatile unsigned*)0xc1104e04 = (delay&7)<<8;
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

typedef unsigned short uint16;
#define TDATA16F (uint16)0xffff
#define TDATA16A (uint16)0xaaaa
#define TDATA165 (uint16)0x5555

static unsigned leng = 0x800000;

static void ddr_write(void *buff, unsigned m_length)
{
    uint16 *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (uint16 *)buff;
    
    while(m_len)
    {
        for(j=0;j<16;j++)
        {
            if(m_len >= 64)
                n = 32;
            else
                n = m_len>>1;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA16F;
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        *(p+i) = 0;
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        *(p+i) = TDATA16A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA165;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = (uint16)(1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = (uint16)(~(1<<j));
                        break;
                }
            }
            
            if(m_len > 64)
            {
                m_len -= 64;
                p += 32;
            }
            else
            {
                p += (m_len>>1);
                m_len = 0;                
                break;
            }
        }
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    uint16 *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (uint16 *)buff;
    
    while(m_len)
    {
        for(j=0;j<16;j++)
        {
            if(m_len >= 64)
                n = 32;
            else
                n = m_len>>1;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA16F)
                            printf("Error1 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), TDATA16F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error2 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        if(*(p+i) != TDATA16A)
                            printf("Error3 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), TDATA16A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA165)
                            printf("Error4 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), TDATA165);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != (uint16)(1<<j))
                            printf("Error5 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != (uint16)(~(1<<j)))
                            printf("Error6 data [0x%04x] at 0x%08x[0x%04x]\n", *(p+i), (unsigned)(p+i), ~(1<<j));
                        break;
                }
            }
            
            if(m_len > 64)
            {
                m_len -= 64;
                p += 32;
            }
            else
            {
                p += (m_len>>1);
                m_len = 0;
                break;
            }
        }
    }
}

void ddr_test()
{    
    unsigned i;

   
    test_buff = (unsigned *)AVMem_malloc(TEST_SIZE);
    if(test_buff)
        printf("Malloc memory OK.\n");
    else
    {
        printf("Malloc memory failed.Exit test.\n");
        return;
    }
    
    for(i=0;i<18;i++)
    {   
        Set_DDR_Delay(3);
        My_Set_DDR_CLK(arr_clk[i][0],arr_clk[i][1]);
        
        printf("DDR CLK is %fMhz, Delay is %u\n", My_Get_DDR_CLK(), Get_DDR_Delay());

        memset(test_buff, 0, TEST_SIZE);
        printf("start write.\n");
        ddr_write(test_buff, TEST_SIZE);
        printf("start read 1.\n");
        ddr_read(test_buff, TEST_SIZE);
        printf("start read 2.\n");
        ddr_read(test_buff, TEST_SIZE);
        printf("start read 3.\n");
        ddr_read(test_buff, TEST_SIZE);        
    }
    
    while(1);    
}
#endif
