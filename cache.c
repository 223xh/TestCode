#define CACHE_LINE_MASK 0xffffffe0
#define IC_LIL_REG 0x13
#define IC_CTRL_REG 0x11
#define IC_CTRL_SB 0x8
#define CACHE_LINE_LENGTH 0x20

int icache_lock(unsigned startAddr, unsigned endAddr)
{
	unsigned s= startAddr&CACHE_LINE_MASK;
	_ASM("flag 0");
	_invalidate_icache();
	do {
		_sr(s,IC_LIL_REG);
		if((_lr(IC_CTRL_REG)&IC_CTRL_SB)==0)
		{
			return -1;
		}
		s += CACHE_LINE_LENGTH;
	}while (s<=endAddr);
	_ASM("jl adc_check");
	return 0;
}
