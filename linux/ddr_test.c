#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#define TDATA32F 0xffffffff
#define TDATA32A 0xaaaaaaaa
#define TDATA325 0x55555555

static unsigned leng = 0x800000;

static void ddr_write(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        *(p+i) = TDATA32F;
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
                        *(p+i) = TDATA32A;
                        break;
                    case 7:
                    case 15:
                    case 24:
                        *(p+i) = TDATA325;
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        *(p+i) = 1<<j;
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        *(p+i) = ~(1<<j);
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
                m_len = 0;                
                break;
            }
        }
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    unsigned *p;
    unsigned i, j, n;
    unsigned m_len = m_length;
    
    p = (unsigned *)buff;
    
    while(m_len)
    {
        for(j=0;j<32;j++)
        {
            if(m_len >= 128)
                n = 32;
            else
                n = m_len>>2;
            
            for(i = 0; i < n; i++)
            {
                switch(i)
                {
                    case 0:
                    case 9:
                    case 14:
                    case 25:
                    case 30:
                        if(*(p+i) != TDATA32F)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32F);
                        break;
                    case 1:
                    case 6:
                    case 8:
                    case 17:
                    case 22:
                        if(*(p+i) != 0)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 0);
                        break;
                    case 16:
                    case 23:
                    case 31:                    
                        if(*(p+i) != TDATA32A)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA32A);
                        break;
                    case 7:
                    case 15:
                    case 24:
                        if(*(p+i) != TDATA325)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), TDATA325);
                        break;
                    case 2:
                    case 4:
                    case 10:
                    case 12:
                    case 19:
                    case 21:
                    case 27:
                    case 29:
                        if(*(p+i) != 1<<j)
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), 1<<j);
                        break;
                    case 3:
                    case 5:
                    case 11:
                    case 13:
                    case 18:
                    case 20:
                    case 26:
                    case 28:
                        if(*(p+i) != ~(1<<j))
                            printf("Error data [0x%08x] at offset 0x%08x[0x%08x]\n", *(p+i), (unsigned)(m_length - m_len - i), ~(1<<j));
                        break;
                }
            }
            
            if(m_len > 128)
            {
                m_len -= 128;
                p += 32;
            }
            else
            {
                p += (m_len>>2);
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
