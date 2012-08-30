#include <includes.h>
#include <math.h>

#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

void Plus_MPEG_CLK()
{
    rd(0xc1104168) += 1;
}

void Minus_MPEG_CLK()
{
    rd(0xc1104168) -= 1;
}

void My_Set_MPEG_CLK(unsigned pll, unsigned div)
{
    unsigned tmp;
    
    tmp = rd(0xc1104168);
    tmp &= ~(0x7fff);
    tmp |= (0x7fff & pll);
    wr(0xc1104168, tmp);
    
    tmp = rd(0xc1104174);
    tmp &= ~0x7f;
    tmp |= (div & 0x7f);
    wr(0xc1104174, tmp);
}

unsigned My_Get_MPEG_CLK(void)
{
    unsigned m, n, div, od, clk;
    
    m = rd(0xc1104168) & 0x1ff;
    n = ((rd(0xc1104168)>>9) & 0x1f);
    od = ((rd(0xc1104168)>>14) & 0x1);    
    div = rd(0xc1104174) & 0x7f;
    
    clk = 24 * m / n / (div + 1) / (unsigned)pow(2.0, (double)od);
    
    return clk;
}