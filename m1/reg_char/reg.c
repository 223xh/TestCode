#include <linux/module.h>        //同kernel.h,最基本的内核模块头文件
#include <linux/kernel.h>  //同module.h,最基本的内核模块头文件
#include <linux/sched.h>  //这里包含了进行正确性检查的宏
#include <linux/fs.h>     //文件系统所必需的头文件
#include <asm/uaccess.h>  //这里包含了内核空间与用户空间进行数据交换时的函数宏
//#include <asm/io.h>      //I/O访问

MODULE_AUTHOR("Eric Xie");
MODULE_DESCRIPTION("Physical Register Driver");
MODULE_LICENSE("Dual BSD/GPL");

int reg_major=0;                 //主设备号
static int Device_Open=0;

#define DEVICE_NAME "REGDev" 

static int reg_open(struct inode *inode, struct file *file)
{//每当应用程序用open打开设备时，此函数被调用
  //printk (KERN_ALERT"device_open(%p,%p)\n", inode, file);
  if (Device_Open)
    return -EBUSY;      //同时只能由一个应用程序打开
  Device_Open++;
  
  try_module_get(THIS_MODULE);//2.6
  return 0;
}
static int reg_release(struct inode *inode, struct file *file)
{//每当应用程序用close关闭设备时，此函数被调用
  //printk (KERN_ALERT"device_release(%p,%p)\n", inode, file);
  Device_Open --;
  
  module_put(THIS_MODULE);//2.6
  
  return 0;
}
static ssize_t  reg_read (struct file *f,char *buf,size_t size,loff_t *off)
{//每当应用程序用read访问设备时，此函数被调用    
    unsigned addr;
    unsigned val;
#ifdef DEBUG
    printk(KERN_ALERT"reg_read is called. User buffer is %p,size is %d\n",buf,size);
#endif
    
    if (!access_ok(VERIFY_WRITE,buf,size)) //2.6
        return -EFAULT;
        
    get_user(addr, (unsigned*)buf);
    
    val = *(volatile unsigned*)addr;
    
    if(put_user(val, (unsigned*)(buf+4))){    //写数据到用户空间
        printk(KERN_ALERT"Put register failed\n");
        return -EINVAL;
    }

    return 8;
}
static ssize_t reg_write (struct file *f,const char *buf, size_t size,loff_t *off)
{//每当应用程序用write访问设备时，此函数被调用
    unsigned addr;
    unsigned val;
#ifdef DEBUG
    printk(KERN_ALERT"reg_write is called. User buffer is %p,size is %d\n",buf,size);
#endif
    
    if (!access_ok(VERIFY_WRITE,buf,size)) //2.6
        return -EFAULT;
        
    get_user(addr, (unsigned*)buf);
    get_user(val, (unsigned*)(buf+4));
    
    *(volatile unsigned*)addr = val;

    return 8;
}

struct file_operations reg_fops = {
    .owner = THIS_MODULE,
    .read =  reg_read,
    .write = reg_write,
    .open = reg_open,
    .release = reg_release
};

static int init_reg_module(void)
{//每当装配设备驱动程序时，系统自动调用此函数
    int result;
    result = register_chrdev(reg_major,DEVICE_NAME,&reg_fops); 
    if (result < 0){
        printk(KERN_ALERT"Register reg module failed\n");
        return result;
    }
    if (reg_major == 0) 
        reg_major = result; 
    printk(KERN_ALERT"Register Ok. major-number=%d\n",result);
    return 0;
}

static void cleanup_reg_module(void)
{//每当卸载设备驱动程序时，系统自动调用此函数
    printk(KERN_ALERT"reg module unload\n");
    unregister_chrdev(reg_major, DEVICE_NAME);
}

module_init(init_reg_module);
module_exit(cleanup_reg_module);
