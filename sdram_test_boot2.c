#define SDRAM_TEST

#ifdef SDRAM_TEST
    #ifndef INT32U
        #define INT32U unsigned int
    #endif
    
    #define MEMSIZE     (0x200000)           //2M
    //#define CODESIZE    (0x80000)            //512K           //(0x21000)           //132K   
    #define FILLDATA    (0x55555555)
    #define BASEADDR    (0x01e00000)
#endif

#ifdef SDRAM_TEST
    register INT32U memSize = MEMSIZE;
    //register INT32U codeSize = CODESIZE;
    register INT32U baseAddr = BASEADDR;
    register INT32U sFlag = 0;//0xFF;
    register INT32U fData = FILLDATA;
    register INT32U numWR = 0;
    register INT32U numErr = 0;
    register INT32U offSet = 0;
    while(1)
    {
        if(sFlag == 0)
        {
            numWR = 0;
            numErr = 0;
            ++sFlag;
        }
        else if(sFlag == 1)
        {
            if(numWR < (memSize >> 3))
            {                
                *(volatile unsigned *) (baseAddr + (1 - offSet & 1) * (memSize >> 1) + (numWR << 2)) = fData;
                ++numWR;
            }
            else
            {
                ++sFlag;                
                numWR = 0;
            }
        }
        else if(sFlag == 2)
        {
            if(numWR < (memSize >> 3))
            {                
                if(*(volatile unsigned *) (baseAddr + (1 - offSet & 1) * (memSize >> 1) + (numWR << 2)) != fData)
                {                    
                    ++numErr;
                }                
                ++numWR;
            }
            else
            {
                sFlag = 0xFF;                
            }
        }
    }
#endif