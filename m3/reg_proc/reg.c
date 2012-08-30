#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#include <mach/am_regs.h>

#include <linux/spinlock.h>

MODULE_AUTHOR("Eric Xie");
MODULE_DESCRIPTION("Physical Register Driver");
MODULE_LICENSE("Dual BSD/GPL");

#define MAX_COMMAND_LENGTH 32//PAGE_SIZE
static struct proc_dir_entry *proc_entry;
static char *command;  // Space for command strings

static char *usage = "Usage:echo rc|rp|rx|rh|rm address\n\
      echo wc|wp|wx|wh|wm address value\n\
      r:read w:write c:CBUS p:APB x:AXI h:AHB m:MPEG\n";

static ssize_t reg_write( struct file *filp, const char __user *buff,
                          unsigned long len, void *data )
{
    int n = 0;
    char *ps, *token;
    char *parm[3];
    unsigned int addr = 0, val = 0, retval = 0;
    unsigned ddr_pll;
    char flag_quiet = 0, flag_irq = 0;
    unsigned long iflags;
    
    spinlock_t spinlock = SPIN_LOCK_UNLOCKED;

    if (len > MAX_COMMAND_LENGTH - 1)
    {
        printk(KERN_INFO "Register: command is too long!\n");
        return -ENOSPC;
    }

    memset( command, 0, MAX_COMMAND_LENGTH );

    if (copy_from_user( command, buff, len ))
    {
        return -EFAULT;
    }

    ps = command;
    while (1)
    {
        token = strsep(&ps, " ");
        if (token == NULL)
            break;

        if(n < 3)
            parm[n++] = token;
        else
            break;
    }

    if(!n)
        goto usage;

    if ((parm[0][0] == 'r'))
    {
        if(n < 2)
        {
            printk ("Error: Lose some paraments!\n");
            goto usage;
        }

        addr = simple_strtol(parm[1], NULL, 0);

        switch (parm[0][1])
        {
        case 'c':
            retval = READ_CBUS_REG(addr);
            break;
        case 'p':
            retval = READ_APB_REG(addr);
            break;
        case 'a':
            addr = addr << 2;
            retval = READ_APB_REG(addr);
            break;
        case 'x':
            retval = READ_AXI_REG(addr);
            break;
        case 'h':
            retval = READ_AHB_REG(addr);
            break;
        case 'm':
            retval = READ_MPEG_REG(addr);
            break;
        default:
            printk ("Error: Invalid command!\n");
            goto usage;
            break;
        }

        printk("<%s>0x%x --> 0x%x\n", parm[0], addr, retval);
    }
    else if ((parm[0][0] == 'w'))
    {
        if(n < 3)
        {
            printk ("Error: Lose some paraments!\n");
            goto usage;
        }

        addr = simple_strtol(parm[1], NULL, 0);
        val = simple_strtol(parm[2], NULL, 0);

        spin_lock_irqsave(&spinlock, iflags);
        switch (parm[0][1])
        {
        case 'c':
            retval = WRITE_CBUS_REG(addr, val);
            break;
        case 'p':
            retval = WRITE_APB_REG(addr, val);
            break;
        case 'a':
            addr = addr << 2;
            retval = WRITE_APB_REG(addr, val);
            break;
        case 'x':
            retval = WRITE_AXI_REG(addr, val);
            break;
        case 'h':
            retval = WRITE_AHB_REG(addr, val);
            break;
        case 'm':
            retval = WRITE_MPEG_REG(addr, val);
            break;
        default:
            printk ("Error: Invalid command!\n");
            goto usage;
            break;
        }
        
        spin_unlock_irqrestore(&spinlock, iflags);
        
        printk("<%s>0x%x <-- 0x%x\n", parm[0], addr, retval);
    }
    else if ((parm[0][0] == 'd'))
    {
        if(n < 2)
        {
            printk ("Error: Lose some paraments!\n");
            goto usage;
        }
        
        #ifdef CONFIG_ARCH_MESON6
            ddr_pll = READ_CBUS_REG(0x1068) & 0x7fffffff;
        #else
            ddr_pll = READ_CBUS_REG(0x1068);
        #endif
        
        if(parm[1][0] == '+')
            ++ddr_pll;
        else if(parm[1][0] == '-')
            --ddr_pll;
        else{
            printk("Invalid parament '%s', cancel set ddr PLL\n", parm[1]);
            return 1;
        }
        
        if(strstr(parm[2], "q"))
            flag_quiet = 1;
        else if(strstr(parm[2], "i"))
            flag_irq = 1;
        
        if(flag_irq)
            spin_lock_irqsave(&spinlock, iflags);
            
        WRITE_CBUS_REG(0x1068, ddr_pll);
        
        if(flag_irq)
            spin_unlock_irqrestore(&spinlock, iflags);
        
        msleep(1);
        
        if(!flag_quiet)
            printk("DDR PLL is set to 0x%x\n", READ_CBUS_REG(0x1068));
    }
    else
    {
        printk("Invalid command!\n");
        goto usage;
    }

    return len;

usage:
    printk("%s", usage);
    return 0;
}
/*
static int reg_read( char *page, char **start, off_t off,
                     int count, int *eof, void *data )
{

    return 0;
}
*/
static int init_reg_module( void )
{
    int ret = 0;
    command = (char *)vmalloc( MAX_COMMAND_LENGTH );
    if (!command)
    {
        ret = -ENOMEM;
    }
    else
    {
        //memset( command, 0, MAX_COMMAND_LENGTH );
        proc_entry = create_proc_entry( "reg", 0644, NULL );
        if (proc_entry == NULL)
        {
            ret = -ENOMEM;
            vfree(command);
            printk(KERN_INFO "Register: Couldn't create proc entry\n");
        }
        else
        {
            //proc_entry->read_proc = reg_read;
            proc_entry->write_proc = reg_write;
            printk(KERN_INFO "Register: Module loaded.\n");
        }
    }
    return ret;
}

static void cleanup_reg_module( void )
{
    remove_proc_entry("reg", NULL);
    vfree(command);
    printk(KERN_INFO "Register: Module unloaded.\n");
}

module_init( init_reg_module );
module_exit( cleanup_reg_module );

