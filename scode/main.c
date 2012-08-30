#include "main.h"
#include "nand.h"
#include "spi.h"

#ifdef NAND
static unsigned char nand_id[MAX_ID_LEN];
#endif

void delay_ms(unsigned ms)
{
    unsigned val;
    
    val = ((C_TIMER_MUX>>8) & 3);
    if(val == 1)
        ms = ms * 1000;
    else if(val == 2)
        ms = ms * 100;
    else if(val == 3)
        ms = ms * 10;
        
    C_TIMER_E = 0;
    while(C_TIMER_E < ms);
}

static void System_Init()
{
    REG(MPEG_PLL_CNTL) = 0xc4e;
    REG(MPEG_CLOCK_CNTL) = 0x181;
    
    REG(DRAM_MASK_0) = 0x8000;
    REG(DRAM_MASK_1) = 0x3b0e;
    REG(DRAM_CTRL_REG1) = 0x500;
}

void main(void)
{    
    DISABLE_WATCHDOG();
    System_Init();
#ifdef NAND
    Nand_Init();
    Nand_Read_ID(nand_id);
    Nand_ID_Output(nand_id);
    Nand_Analyse_Info(nand_id);
    //Nand_Reset();
    Nand_Block_Erase(1, 2, 3);//all
    //Nand_Block_Erase(0, 2, 3);//boot
#endif

#ifdef SPI
    Init_SPI();
    SPI_Get_ID();
    SPI_Chip_Erase();
#endif
    while(1);
}