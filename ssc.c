#define wr(reg, val) *(volatile unsigned *)(reg) = (val)
#define rd(reg) (*(volatile unsigned *)(reg))

#define MPEG_PLL_CONTROL2 0x01100580
#define MPEG_PLL_CONTROL3 0x01100584
#define VIDEO_PLL_CONTROL2 0x01100590
#define VIDEO_PLL_CONTROL3 0x01100594

static void set_mpeg_pll_ssc(unsigned cmod_clk_sel, unsigned pll_cmod_ctr)
{	
	unsigned temp = rd(MPEG_PLL_CONTROL2);
	temp &= ~(0xf<<12 | 0xf<<6);
	temp |= (((cmod_clk_sel & 0xf)<<12) | ((pll_cmod_ctr & 0xf)<<6));
	wr(MPEG_PLL_CONTROL2, temp);
}

static void set_video_pll_ssc(unsigned cmod_clk_sel, unsigned pll_cmod_ctr)
{
	unsigned temp = rd(VIDEO_PLL_CONTROL2);
	temp &= ~(0xf<<12 | 0xf<<6);
	temp |= (((cmod_clk_sel & 0xf)<<12) | ((pll_cmod_ctr & 0xf)<<6));
	wr(VIDEO_PLL_CONTROL2, temp);
}

void Set_MPEG_SSC(unsigned char level)
{
    switch(level)
    {
        case 1:
            set_mpeg_pll_ssc(8,4);
            break;
        case 2:
            set_mpeg_pll_ssc(8,6);
            break;
        case 3:
            set_mpeg_pll_ssc(8,8);
            break;
        case 4:
            set_mpeg_pll_ssc(4,6);
            break;
        case 5:
            set_mpeg_pll_ssc(2,9);
            break;
        default:
            break;        
    }
    
    wr(MPEG_PLL_CONTROL3, 0x103);
}

void Set_VIDEO_SSC(unsigned char level)
{
    switch(level)
    {
        case 1:            
            set_video_pll_ssc(8,7);
            break;
        case 2:            
            set_video_pll_ssc(4,11);
            break;
        case 3:            
            set_video_pll_ssc(2,7);
            break;
        case 4:
            set_video_pll_ssc(2,9);
            break;
        case 5:
            set_video_pll_ssc(2,12);
            break;
        default:
            break;        
    }
    
    wr(VIDEO_PLL_CONTROL3, 0x103);
}

void Set_SSC(unsigned char level)
{
    Set_VIDEO_SSC(level);
    Set_MPEG_SSC(level);
}