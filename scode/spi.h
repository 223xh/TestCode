#ifndef _SPI_H
#define _SPI_H

#define PERIPHS_SPI_FLASH_CMD        0x1200500
#define PERIPHS_SPI_FLASH_ADDR       0x1200504
#define PERIPHS_SPI_FLASH_CTRL       0x1200508
#define PERIPHS_SPI_FLASH_CTRL1      0x120050c
#define PERIPHS_SPI_FLASH_STATUS     0x1200510
#define PERIPHS_SPI_FLASH_CTRL2      0x1200514
#define PERIPHS_SPI_FLASH_C0         0x1200520
#define PERIPHS_SPI_FLASH_C1         0x1200524
#define PERIPHS_SPI_FLASH_C2         0x1200528
#define PERIPHS_SPI_FLASH_C3         0x120052c
#define PERIPHS_SPI_FLASH_C4         0x1200530
#define PERIPHS_SPI_FLASH_C5         0x1200534
#define PERIPHS_SPI_FLASH_C6         0x1200538
#define PERIPHS_SPI_FLASH_C7         0x120053c

void Init_SPI();
void SPI_Get_ID();
void SPI_Chip_Erase();

#endif