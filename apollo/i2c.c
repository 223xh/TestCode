#include "ioapi.h"

#ifndef SOFT_I2C_FLAG
#include "Drivers/i2c/hard_i2c.h"
#else 
#include "Drivers/i2c/soft_i2c.h"
#endif

int I2C_device_fd = -1;
int i2c_handle_flag = 0;

int I2CRead(unsigned char id, unsigned bytes, unsigned addr, unsigned char *buff );
int I2CWrite(unsigned char id, unsigned bytes, unsigned addr, unsigned char *buff );
void Init_I2C_para(void)
{
	
	//unsigned char w_data[8] = {1,2,3,4,5,6,7,8,};
	unsigned char w_data[8] = {'a','b','c','d','e','f','g','h',};
	unsigned char r_data[8] = {0,0,0,0,0,0,0,0,};
	int ret;
	
	*(volatile unsigned *)0xc12000b0 &= ~(0xff<<17);
	*(volatile unsigned *)0xc12000b0 |= (0x88<<17);

#ifndef SOFT_I2C_FLAG
      hard_i2c_para_t temp_i2c_con = {
    	              0,                  //unsigned            magic;
                    "/dev/hard_i2c",    //char       *device_name;
                    0xc2,               //unsigned char       device_id;
                    0xff,               //unsigned char       id_mask;
                    1,                  //unsigned char       i2c_flag    
                    0x10000,                //unsigned            max_addr;
                    2,                  //unsigned            delay;
                    100    //400,       //unsigned            requency
                };
#else
     soft_i2c_para_t temp_i2c_con = {
                      1,    //unsigned            magic;
                    "/dev/soft_i2c",   //char       *device_name
                      0xa0,   //unsigned char       device_id;
                    0xff,   //unsigned char       id_mask;
                    0,      //unsigned char       i2c_flag    
                    512,    //unsigned            max_addr;
                    70,     //unsigned            delay; requency is about 80khz
                    0x012000A0,   //unsigned            scl_en_reg;  //FEC_D_0 is muxed to scl
                    0x20000000,   //unsigned            scl_en_mask; 
        
                    0x012000A4,   //unsigned            scl_output_reg; 
                    0x20000000,   //unsigned            scl_output_mask; 

                    0x012000A8,   //unsigned            scl_input_reg;         
                    0x20000000,   //unsigned            scl_input_mask;  
      
                    0x012000A0,   //unsigned            sda_en_reg;                  //FEC_D_1 is muxed to sda
                    0x10000000,   //unsigned            sda_en_mask; 
       
                    0x012000A4,   //unsigned            sda_output_reg;      
                    0x10000000,   //unsigned            sda_output_mask; 
    
                    0x012000A8,   //unsigned            sda_input_reg;         
                    0x10000000,   //unsigned            sda_input_mask;   
                };
#endif
        
        if(I2C_device_fd < 0)
           I2C_device_fd = open(temp_i2c_con.device_name, O_RDWR);
        
        if(I2C_device_fd > 0)
        {
            ioctl(I2C_device_fd, INIT_I2C_PIN_MUX, &temp_i2c_con);
           
            ret = I2CWrite(0xa0, 8, 0, w_data );
            AVTimeDly(200);
            ret = I2CRead(0xa0, 8, 0, r_data );
            printf("Read data is [%s]\n",r_data);
        }
        else
            printf("Can not open i2c!\n");
}

static void ModifyI2CID(unsigned char id)
{
	ioctl(I2C_device_fd, MODIFY_DEVICE_ID, id); 		
}


static void ModifyI2CFlag(unsigned char flag)
{
	ioctl(I2C_device_fd, MODIFY_I2C_FLAG, flag); 
}


static void ModifyI2CDelay(unsigned delay)
{
	ioctl(I2C_device_fd, MODIFY_I2C_DELAY, delay); 
}

int I2CRead(unsigned char id, unsigned bytes, unsigned addr, unsigned char *buff )
{
   int i2c_rw_flag = -1 ;
         	   
   if((I2C_device_fd > 0) && (i2c_handle_flag == 0))
   	{
   	    i2c_handle_flag = 1; 
        ModifyI2CID(id);
	      i2c_rw_flag = read(I2C_device_fd, buff , bytes | (addr << 16));
	      i2c_handle_flag = 0;
	 }
	
    return i2c_rw_flag;
}

int I2CWrite(unsigned char id, unsigned bytes, unsigned addr, unsigned char *buff )
{
    int i2c_rw_flag = -1 ;
          	   
    if((I2C_device_fd > 0) && (i2c_handle_flag == 0))
    {
   	    i2c_handle_flag = 1; 
        ModifyI2CID(id);
	      i2c_rw_flag = write(I2C_device_fd, buff , bytes | (addr << 16));
	      i2c_handle_flag = 0;
    }
		
    return i2c_rw_flag;
}