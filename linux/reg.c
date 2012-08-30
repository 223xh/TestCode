#include <linux/module.h>        //ͬkernel.h,��������ں�ģ��ͷ�ļ�
#include <linux/kernel.h>  //ͬmodule.h,��������ں�ģ��ͷ�ļ�
#include <linux/sched.h>  //��������˽�����ȷ�Լ��ĺ�
#include <linux/fs.h>     //�ļ�ϵͳ�������ͷ�ļ�
#include <asm/uaccess.h>  //����������ں˿ռ����û��ռ�������ݽ���ʱ�ĺ�����
//#include <asm/io.h>      //I/O����

MODULE_AUTHOR("Eric Xie");
MODULE_DESCRIPTION("Physical Register Driver");
MODULE_LICENSE("Dual BSD/GPL");

int reg_major=0;                 //���豸��
static int Device_Open=0;

#define DEVICE_NAME "REGDev" 

static int reg_open(struct inode *inode, struct file *file)
{//ÿ��Ӧ�ó�����open���豸ʱ���˺���������
  //printk (KERN_ALERT"device_open(%p,%p)\n", inode, file);
  if (Device_Open)
    return -EBUSY;      //ͬʱֻ����һ��Ӧ�ó����
  Device_Open++;
  
  try_module_get(THIS_MODULE);//2.6
  return 0;
}
static int reg_release(struct inode *inode, struct file *file)
{//ÿ��Ӧ�ó�����close�ر��豸ʱ���˺���������
  //printk (KERN_ALERT"device_release(%p,%p)\n", inode, file);
  Device_Open --;
  
  module_put(THIS_MODULE);//2.6
  
  return 0;
}
static ssize_t  reg_read (struct file *f,char *buf,size_t size,loff_t *off)
{//ÿ��Ӧ�ó�����read�����豸ʱ���˺���������    
    unsigned addr;
    unsigned val;
#ifdef DEBUG
    printk(KERN_ALERT"reg_read is called. User buffer is %p,size is %d\n",buf,size);
#endif
    
    if (!access_ok(VERIFY_WRITE,buf,size)) //2.6
        return -EFAULT;
        
    get_user(addr, (unsigned*)buf);
    
    val = *(volatile unsigned*)addr;
    
    if(put_user(val, (unsigned*)(buf+4))){    //д���ݵ��û��ռ�
        printk(KERN_ALERT"Put register failed\n");
        return -EINVAL;
    }

    return 8;
}
static ssize_t reg_write (struct file *f,const char *buf, size_t size,loff_t *off)
{//ÿ��Ӧ�ó�����write�����豸ʱ���˺���������
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
{//ÿ��װ���豸��������ʱ��ϵͳ�Զ����ô˺���
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
{//ÿ��ж���豸��������ʱ��ϵͳ�Զ����ô˺���
    printk(KERN_ALERT"reg module unload\n");
    unregister_chrdev(reg_major, DEVICE_NAME);
}

module_init(init_reg_module);
module_exit(cleanup_reg_module);
