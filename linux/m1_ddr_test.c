#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include <sys/time.h>

/*
#define RAND1 0xc1108100
#define RAND2 0xc1108104

#define reg(a) (*(volatile unsigned*)(a))
*/
static unsigned leng = 158;

static unsigned td[4][4]={
                            {0x00000000, 0xffffffff, 0x00000000, 0xffffffff},
                            {0x55555555, 0xaaaaaaaa, 0x55555555, 0xaaaaaaaa},
                            {0xdd22ee11, 0x7788bb44, 0xdd22ee11, 0x7788bb44},
                            {0x55aa00ff, 0xaa0055ff, 0x55ffaa00, 0xff00aa55},
                         };

static void delay_10us(unsigned count)
{
    int i ;
    for(; count > 0 ; count--) 
      for(i=450; i > 0; i--) 
        {;}
}

static void ddr_write(void *buff, unsigned m_length)
{
    unsigned m_len, i, j;
    unsigned *p;
    p = (unsigned*)buff;
    
    m_len = m_length * 1024 * 1024;
    
    for(i=0;i<4;i++) {
        for(j=0;j<(m_len>>4);j+=4) {
            *(p+j)   = td[i][0];
            *(p+j+1) = td[i][1];
            *(p+j+2) = td[i][2];
            *(p+j+3) = td[i][3];
        }
        p += (m_len>>2);
    }
}

static void ddr_read(void *buff, unsigned m_length)
{
    unsigned m_len, i, j;
    unsigned *p;
    p = (unsigned*)buff;
    
    m_len = m_length * 1024 * 1024;
    
    for(i=0;i<4;i++) {
        for(j=0;j<(m_len>>4);j+=4) {
            if(*(p+j)   != td[i][0]) {
                printf("Error data 0x%08x[0x%08x] at 0x%08x.\n", *(p+j), td[i][0], (unsigned)(p+j));
            }
            if(*(p+j+1) != td[i][1]) {
                printf("Error data 0x%08x[0x%08x] at 0x%08x.\n", *(p+j+1), td[i][0], (unsigned)(p+j+1));
            }
            if(*(p+j+2) != td[i][2]) {
                printf("Error data 0x%08x[0x%08x] at 0x%08x.\n", *(p+j+2), td[i][0], (unsigned)(p+j+2));
            }
            if(*(p+j+3) != td[i][3]) {
                printf("Error data 0x%08x[0x%08x] at 0x%08x.\n", *(p+j+3), td[i][0], (unsigned)(p+j+3));
            }
        }
        p += (m_len>>2);
    }
}

static void ddr_rand_inc_write(void *buff, unsigned m_length, unsigned base, unsigned inc)
{
    unsigned *p = (unsigned*)buff;
    unsigned m_len = m_length * 1024 *1024;
    unsigned i;
    
    for(i=0;i<(m_len>>2);i++) {
        *(p+i) = (unsigned)(base + (inc*i));
    }
}

static void ddr_rand_inc_read(void *buff, unsigned m_length, unsigned base, unsigned inc)
{
    unsigned *p = (unsigned*)buff;
    unsigned m_len = m_length * 1024 *1024;
    unsigned i;
    
    for(i=0;i<(m_len>>2);i++) {
        if(*(p+i) != ((unsigned)(base + (inc*i)))) {
            printf("\nError data 0x%08x[0x%08x] at 0x%08x.", *(p+i), ((unsigned)(base + (inc*i))), (unsigned)(p+i));
        }
    }
}

static void ddr_rand_inc_test(void *buff, unsigned m_length)
{
    unsigned rand_base, rand_inc, i;
    char *p = (char*)buff;
    struct timeval tv;
    
    for(i=0;i<m_length;i++) {
        gettimeofday(&tv,NULL);
        srand((unsigned)tv.tv_sec*1000 + (unsigned)tv.tv_usec/1000);
        rand_base = rand();//reg(RAND1);
        delay_10us(10);
        gettimeofday(&tv,NULL);
        srand((unsigned)tv.tv_sec*1000 + (unsigned)tv.tv_usec/1000);
        rand_inc = rand();//reg(RAND2);
        ddr_rand_inc_write(p, 1, rand_base, rand_inc);
        ddr_rand_inc_read(p, 1, rand_base, rand_inc);
        p += 0x100000;
        printf(">");
        fflush(stdout);
    }
}

int main(int argc,char *argv[])
{
    char *m_buff = 0;
    char *p;
    
    if(argc == 1)
        goto START_TEST;
        
    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 )
        goto END;
        
    leng = strtoul(argv[1],NULL,10);
    if(!leng)
        goto ERRORLEN;
        
    if(leng < 2)
        goto ERRORLEN2;
    
START_TEST:
    
    do {
        
        if(leng < 8) {
            printf("Less than 8Mbytes for test!\n");
            return 3;
        }
        
        m_buff = malloc(leng*1024*1024);
        
        if(m_buff) {
            printf("Malloc %u Mbytes memory[0x%x] ok!\n", leng, (unsigned)m_buff);
            break;
        }
    }while(leng--);
   
    printf("Start normal write...\n");
    ddr_write(m_buff, 1);
    printf("Start normal read...\n");
    ddr_read(m_buff, 1);
    p = m_buff + 4*1024*1024;
    printf("Start rand inc test...\n");
    ddr_rand_inc_test(p, leng-4);
    printf("\nTest done.\n");
    
    free(m_buff);
   
    
    return 0;

ERRORLEN:
    printf("Error format at 'length'.\n");
    return 1;
    
ERRORLEN2:
    printf("Error 'length', 8 Mbytes at least.\n");
    return 2;
    
END:
    printf("    %s [length]  -  Test DDR 'length' bytes.Default is 158M bytes\n", argv[0]);
    return 0;
}