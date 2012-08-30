#include "nand.h"

static NandInfo thisNand;

static void Nand_Check_Busy()
{
    do
    {
        delay_ms(1);
    }
    while(!NAND_CHECK_BUSY);
}

void Nand_Init()
{
    REG(PERIPHS_PIN_MUX_0)    |= (1 << 0);
    REG(NAND_CONFIG)	|=	(1 << 7)|			// NAND connected to DCU1
							  (1 << 6) |
								(1 << 3) |			// Large page (2k)
								(1 << 2) |			//	3 ALE's
								(0 << 1);			  // 8-bit strange	
}

void Nand_Reset()
{
    Nand_Check_Busy();
    NAND_SEND_COMMAND(NAND_CMD_RESET);
    Nand_Check_Busy();
}

void Nand_Read_ID(uchar *data)
{
    unsigned *p;
    uchar i;
    
    memset(data, 0, MAX_ID_LEN);
    p = (unsigned*)data;
    
    NAND_SEND_COMMAND(NAND_CMD_READID);
    NAND_SEND_ADDR(0x00);
    for (i=0; i<MAX_ID_LEN/sizeof(unsigned); i++)
    {
        *p++=NAND_GET_DATA();
    }
}

void Nand_ID_Output(uchar *data)
{
    uchar i;
    uchar *p;
    
    p = data;
    
    printf("Nand ID is:\n");
    for(i=0; i<MAX_ID_LEN-3; i++)
    {        
        printf("0x%x\n", *(p++));
    }
}

void Nand_Analyse_Info(uchar *id)
{
    thisNand.page_shift = (id[3] & 3) + 10;
    thisNand.block_shift = ((id[3]>>4) & 3) + 16;
    thisNand.chip_shift = (((id[4]>>4) & 7) + 23) + ((id[4]>>2) & 3);
    thisNand.block_num = 1<<(thisNand.chip_shift - thisNand.block_shift);
    thisNand.plane_num = ((id[4]>>2) & 3) + 1;
}

void Nand_Block_Erase(uchar flag, uchar colSize, uchar rowSize)
{
    ushort i, bn;
    unsigned rowAddr;
    unsigned char j, status;
    bn = 1;
    i = 0;
    
    if(flag)
        bn = thisNand.block_num;
        
    printf("Start erasing, please wait");
    fflush(stdout);
        
    while(i<bn)
    {
        rowAddr = i<<(thisNand.block_shift - thisNand.page_shift);
        NAND_SEND_COMMAND(NAND_CMD_ERASE1);
        for(j=0; j<rowSize; j++)
        {
            NAND_SEND_ADDR((rowAddr>>(j*8)) & 0xff);
            //printf("send addr[0x%x]\n",(rowAddr>>(j*8)) & 0xff);
        }

        if(thisNand.plane_num > 1)
        {
            ++i;
            rowAddr = i<<(thisNand.block_shift - thisNand.page_shift);
            NAND_SEND_COMMAND(NAND_CMD_ERASE1);
            for(j=0; j<rowSize; j++)
            {
                NAND_SEND_ADDR((rowAddr>>(j*8)) & 0xff);
                //printf("send addr[0x%x]\n",(rowAddr>>(j*8)) & 0xff);
            }
        }
        
        NAND_SEND_COMMAND(NAND_CMD_ERASE2);
        
        Nand_Check_Busy();
        
        NAND_SEND_COMMAND(NAND_CMD_STATUS);
        
        Nand_Check_Busy();
        
        status = NAND_GET_DATA();
        
        if(status & 1)
            printf("\n<Erase block [%u] failed!>\n", i);
        else
        {
            if(i % 64 == 1)
            {
                printf(".");
                fflush(stdout);
            }
        }
        
        ++i;
    }
    
    printf("\nErase %u blocks done.\n", i);
}