#include "stdio.h"
#include "string.h"
#include "stdlib.h"
typedef unsigned short uint16;
#define TDATA16F (uint16)0xffff
#define TDATA16A (uint16)0xaaaa
#define TDATA165 (uint16)0x5555

static unsigned leng = 0x800000;

static void ddr_write(void *buff, unsigned m_length)
{
    uint16 *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (uint16 *)buff;
    
    while(m_len)
    {
        for(j=0;j<16;j++)
        {
            if(m_len >= 64)
                n = 32;
            else
                n = m_len>>1;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA16F;
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        *(p+i) = 0;
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        *(p+i) = TDATA16A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA165;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = (uint16)(1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = (uint16)(~(1<<j));
                        break;
                }
            }
            
            if(m_len > 64)
            {
                m_len -= 64;
                p += 32;
            }
            else
            {
                p += (m_len>>1);
                m_len = 0;                
                break;
            }
        }
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    uint16 *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (uint16 *)buff;
    
    while(m_len)
    {
        for(j=0;j<16;j++)
        {
            if(m_len >= 64)
                n = 32;
            else
                n = m_len>>1;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA16F)
                            printf("Error1 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA16F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error2 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        if(*(p+i) != TDATA16A)
                            printf("Error3 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA16A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA165)
                            printf("Error4 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA165);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != (uint16)(1<<j))
                            printf("Error5 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != (uint16)(~(1<<j)))
                            printf("Error6 data [0x%04x] at offset 0x%08x[0x%04x]\n", *(p+i), (unsigned)(m_length - m_len - i), ~(1<<j));
                        break;
                }
            }
            
            if(m_len > 64)
            {
                m_len -= 64;
                p += 32;
            }
            else
            {
                p += (m_len>>1);
                m_len = 0;
                break;
            }
        }
    }
}

int main(int argc,char *argv[])
{
    char *m_buff = 0;
    
    if(argc == 1)
        goto START_TEST;
        
    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 )
        goto END;
        
    leng = strtoul(argv[1],NULL,16);
    if(!leng)
        goto ERRORLEN;
    
START_TEST:
    
    m_buff = malloc(leng);
    if(m_buff)
    {
        printf("Malloc 0x%x bytes memory[0x%x] ok!Start test...\n", leng, (unsigned)m_buff);
        
        printf("Start write...\n");
        ddr_write(m_buff, leng);
        printf("Start read...\n");
        ddr_read(m_buff, leng);
        printf("Start read...\n");
        ddr_read(m_buff, leng);
        printf("Start read...\n");
        ddr_read(m_buff, leng);
        printf("Test done.\n");
        
        free(m_buff);
    }
    else
        printf("Malloc 0x%x bytes memory failed!\n", leng);
    
    return 0;

ERRORLEN:
    printf("Error test 'length', must be hex number!\n");
    return 1;
    
END:
    printf("    %s [length]  -  Test DDR 'length' bytes.Default is 8M bytes\n", argv[0]);
    return 0;
}
