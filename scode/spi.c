#include "spi.h"
#include "main.h"

void Init_SPI()
{
    REG(PERIPHS_SPI_FLASH_CTRL) = 0xea525;
#ifdef WITHOUTHOLD
    REG(PERIPHS_SPI_FLASH_CTRL) &= ~(1<<18);
#endif
    REG(PERIPHS_SPI_FLASH_CTRL) &= ~(1<<17);//Clear AHB
}

void SPI_Get_ID()
{
    unsigned spi_id = 0;
        
    REG(PERIPHS_SPI_FLASH_C0) = 0;
    REG(PERIPHS_SPI_FLASH_CMD) |= 1<<28;//Read Identification (RDID) (9Fh) 
    while(REG(PERIPHS_SPI_FLASH_CMD)!=0);
    
    spi_id = REG(PERIPHS_SPI_FLASH_C0)&0xffffff;
    printf("SPI id is 0x%x\n", spi_id);
}

void SPI_Chip_Erase()
{
    printf("SPI erasing");
    fflush(stdout);
    REG(PERIPHS_SPI_FLASH_CMD) |= (1<<30);	// write enable
    while(REG(PERIPHS_SPI_FLASH_CMD)!=0);
    
    // Chip erase.
    REG(PERIPHS_SPI_FLASH_CMD) |= (1<<22);
    while(REG(PERIPHS_SPI_FLASH_CMD)!=0);
    
    do
    {
        REG(PERIPHS_SPI_FLASH_CMD) |= (1<<27);
        while(REG(PERIPHS_SPI_FLASH_CMD)!=0);
        
        printf(".");
        fflush(stdout);
        //AVTimeDly(500);
        delay_ms(500);
    }
    while(REG(PERIPHS_SPI_FLASH_STATUS)&1);
    
    printf("\nSPI erase done.\n");
}