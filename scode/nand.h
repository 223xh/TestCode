#ifndef _NAND_H
#define _NAND_H

#include "main.h"

#ifdef AML_NIKED3
#define NAND_MANUAL_CLE_REG  0x01200300
#define NAND_MANUAL_ALE_REG  0x01200304
#define NAND_MANUAL_DATA_REG 0x01200308

#define NAND_CONFIG          0x01200580
#else

#endif

#define NAND_CMD_STATUS	0x70
#define NAND_CMD_READID 0x90
#define NAND_CMD_ERASE1	0x60
#define NAND_CMD_ERASE2	0xd0
#define NAND_CMD_RESET  0xff

#define NAND_SEND_COMMAND(a) (REG(NAND_MANUAL_CLE_REG) = (a&0xff))
#define NAND_SEND_ADDR(a)    (REG(NAND_MANUAL_ALE_REG) = (a&0xff))
#define NAND_GET_DATA()      (REG(NAND_MANUAL_DATA_REG))

#define NAND_CHECK_BUSY (REG(NAND_CONFIG)&0x80000000)
#define NAND_CHECK_BUSY1 (NAND_GET_DATA()&(1<<6))

#define MAX_ID_LEN 8

typedef struct{
    uchar page_shift;
    uchar block_shift;
    uchar chip_shift;
    ushort block_num;
    uchar plane_num;
}NandInfo;

void Nand_Init();
void Nand_Read_ID(uchar *data);
void Nand_ID_Output(uchar *data);
void Nand_Analyse_Info(uchar *id);
void Nand_Block_Erase(uchar flag, uchar colSize, uchar rowSize);
void Nand_Reset();

#endif