#define SDRAM_TEST

#ifdef SDRAM_TEST
    #ifndef INT32U
        #define INT32U unsigned int
    #endif
    #ifndef NULL
        #define NULL 0
    #endif
    #define MEMSIZE (1024 * 2048)           //2M
    #define MEMLENGTH  (INT32U)((MEMSIZE>>3)+(MEMSIZE>>4))    //1.5M
    
    INT32U sFlag = 0;//0xFF;
    INT32U numOff = 0;  //offset MAX=13
    INT32U *numMem = NULL;
    INT32U numWR = 0;
    INT32U numErr = 0;
    
    void SdramTest(void);
#endif

#ifdef SDRAM_TEST
    while(1)
    {
        SdramTest();
    }
#endif

#ifdef SDRAM_TEST
    void SdramTest(void)
    {
        if(sFlag == 0)
        {            
            numMem = (INT32U *)mem_alloc(sizeof(INT32U)*MEMLENGTH);
            if(numMem == 0)
            {
                sFlag = 0xEE;                
                mem_free(numMem);
                numMem = NULL;                
            }
            else
            {
                sFlag += 1;                
                memset(numMem, 0, sizeof(INT32U)*(MEMLENGTH));
            }
        }
        else if(sFlag == 1)
        {
            if(numWR < MEMLENGTH)
            {                
                numMem[numWR] = (INT32U)((numWR + 1) << numOff);
                numWR += 1;
            }
            else
            {
                sFlag += 1;                
                numWR = 0;
            }
        }
        else if(sFlag == 2)
        {
            if(numWR < MEMLENGTH)
            {                
                if(numMem[numWR] == (INT32U)((numWR + 1) << numOff))
                {
                    
                }
                else
                {                   
                    numErr += 1;
                }                
                numWR += 1;
            }
            else
            {
                sFlag = 0xFF;                
                numWR = 0;
                
                mem_free(numMem);
                numMem = NULL;
                numErr = 0;
            }
        }
    }
#endif