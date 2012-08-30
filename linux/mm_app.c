#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <termios.h>
#include <assert.h>

#define DEVICE_NAME "/dev/reg"//"REGDev" 

#define DEFAULT_LENGTH 20

#define ERASE_CMD "\b \b"
#define CMD_LEN   10

static char strcmd[CMD_LEN+1];

//static unsigned addr, leng, val;

static int getch( void ) 
{
      int c = 0;
      struct termios org_opts, new_opts;
      int res = 0;
          //-----  store old settings -----------
      res = tcgetattr( STDIN_FILENO, &org_opts );
      assert( res == 0 );
          //---- set new terminal parms --------
      memcpy( &new_opts, &org_opts, sizeof(new_opts) );
      new_opts.c_lflag &= ~( ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL );
      tcsetattr( STDIN_FILENO, TCSANOW, &new_opts );
      c = getchar();
          //------  restore old settings ---------
      res = tcsetattr( STDIN_FILENO, TCSANOW, &org_opts );
      assert( res == 0 );
      return( c );
}

static int getcmd(void)
{
    int c;
    char *p = strcmd;
    unsigned char po = 0;
    
    while(1){
        c = getch();
        
        if((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == 'x' || c == 'X'){
            if((po != 1 || *p != '0') && (c == 'x' || c == 'X')){
                continue;
            }
            
            if(po < CMD_LEN){
                *(p+po) = c;
                printf("%c", c);
                ++po;
            }
        }
        else{
        
            switch(c){
                case '\r':				/* Enter		*/
		            case '\n':
		        	    *(p+po) = '\0';
		        	    printf("\n");
		        	    return po;
		        	  case '\0':
		        	    continue;
		        	  case 0x03:    /* ^C - break		*/
		        	    *(p+po) = '\0';
		        	    return -1;
		        	  case 0x08:				/* ^H  - backspace	*/
		            case 0x7F:				/* DEL - backspace	*/
		              if(po){
		                printf(ERASE_CMD);
		                --po;
		              }
		              break;
		            case 'q':
		            case 'Q':
		              printf("\n");
		              return 17;
		            		        	      
            }
            
        }
    }
}

int main(int argc,char *argv[])
{
    unsigned addr, leng, val;
    unsigned i, w_val;
    int ret;
    char key;
    unsigned char j;
    int fd;
    unsigned char buff[8];
    unsigned *p;
    unsigned reg_val[4];    
    
    p = (unsigned*)buff;
    
    fd = open(DEVICE_NAME, O_RDWR);
    if(fd == -1){
        printf("Open dev failed![%d]\n", fd);
        return 1;
    }
    
    if(argc == 1)
        goto END;
        
    if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 )
        goto END;
        
    if(argc == 2)
        goto ERROR;
    
    if(strcmp(argv[1], "-r") == 0)
    {
        addr = strtoul(argv[2],NULL,16);
        
        if(!addr)
            goto ERRORADDR;
            
        if(argc == 3)
            leng = DEFAULT_LENGTH;
        else
        {
            leng = strtoul(argv[3],NULL,10);
            if(!leng)
                leng = DEFAULT_LENGTH;
        }
        
        i = 0;
        while(i<leng)
        {
            if(leng - i >= 4)
            {
                for(j=0;j<4;j++){
                    *p = addr + 4*(i+j);
                    read(fd, buff, 8);
                    reg_val[j] = *(p+1);
                }
                
                printf("[0x%08x]:  0x%08x  0x%08x  0x%08x  0x%08x\n", \
                addr+i, reg_val[0], reg_val[1], reg_val[2], reg_val[3]);                
                 
                i += 4;
                
                if(i % 80 == 0)
                {
                    printf("Press 'q' to exit, another key to continue.\n");
                    key = getch();
                    if((key == 'q') || (key == 'Q'))
                        break;
                }
            }
            else
            {
                for(j=0;j<3;j++){
                    *p = addr + 4*(i+j);
                    read(fd, buff, 8);
                    reg_val[j] = *(p+1);
                }
                
                printf("[0x%08x]:  0x%08x", addr+4*i, reg_val[0]);
                
                if(i == 2)
                    printf("  0x%08x", reg_val[1]);
                else if(i == 3)
                    printf("  0x%08x  0x%08x", reg_val[1], reg_val[2]);
                    
                printf("\n");
            }
        }
        
        close(fd);
        return 0;
    }
    else if(strcmp(argv[1], "-w") == 0)
    {
        if(argc == 3)
            goto ERROR;
        
        addr = strtoul(argv[2],NULL,16);
        
        if(!addr)
            goto ERRORADDR;
            
        val = strtoul(argv[3],NULL,16);
        
        if(!val)
            goto ERRORADDR;
            
        *p = addr;
        *(p+1) = val;
        write(fd, buff, 8);
        
        close(fd);
        return 0;
    }
    else if(strcmp(argv[1], "-wc") == 0)
    {
        addr = strtoul(argv[2],NULL,16);
        
        if(!addr)
            goto ERRORADDR;
        
        i = 0;
        while(1){
            *p = addr + i;
            read(fd, buff, 8);
            printf("[0x%08x] 0x%08x -> ", *p, *(p+1));
            
            ret = getcmd();
            if(ret == 17){
                break;
            }
            else if(ret == 0){
                goto end_wc;
            }
            
            w_val = strtoul(strcmd,NULL,16);
            
            if(!w_val){
                printf("\n");
                continue;
            }
            
            *(p+1) = w_val;
            write(fd, buff, 8);
end_wc:               
            i += 4;
        }
        
        close(fd);
        return 0;
    }
    else
        goto ERROR;

ERRORADDR:
    printf("Error address value!\n");
    goto END;

ERROR:
    printf("Error command format!\n");
    
END:
    printf("    %s [Parameter] [Address] [Length|Value]\n \
                    [Parameter].\n \
                        -r   Read 'Length' registers from 'Address' \n \
                        -w   Write 'Address' 'Value'\n \
                        -wc  Write form 'Address' continuous\n \
                        -h --help Show help info\n \
                    [Length].Read length.\n \
                    [Value]. Write value.\n", argv[0]);
    
    close(fd);
    return 0;
}
