#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#include <linux/uaccess.h>
#include <asm/uaccess.h>


#define MAX_COOKIE_LENGTH   PAGE_SIZE
#define PROC_FILE_NAME      "my_fortune"

static struct proc_dir_entry *proc_file;

static char* cookie_pot;
static int cookie_index;  
static int next_fortune;


ssize_t fortune_write(struct file *file, const char __user *buff, unsigned long len, loff_t *f_pos);
ssize_t fortune_read(struct file *file, char __user *buff, size_t count, loff_t *f_pos);
static int __init init_fortune_module(void);
static void __exit exit_fortune_module(void);


ssize_t fortune_write(struct file *file, const char __user *buff, unsigned long len, loff_t *f_pos)
{
    int free_space = (MAX_COOKIE_LENGTH - cookie_index) + 1;

    if (free_space < len + 1)
    {
        printk(KERN_ERR "my_fortune: cookie_pot is overflowed");
        return -EFAULT;
    }

    if (copy_from_user(&cookie_pot[cookie_index], buff, len))
    {
        printk(KERN_INFO "my_fortune: copy_from_user error");
        return -EFAULT;
    }

    cookie_index += len + 1;
    cookie_pot[cookie_index - 2] = '\n';
    cookie_pot[cookie_index - 1] = 0;

    return len; 
}

ssize_t fortune_read(struct file *file, char __user *buff, size_t count, loff_t *f_pos)
{
    int  ln = 0;

    if (cookie_index == 0 || *f_pos > 0)
        return 0;

    if (next_fortune >= cookie_index)
        return 0;
    
    ln = strlen(&cookie_pot[next_fortune]);
    copy_to_user(buff, &cookie_pot[next_fortune], ln);

    next_fortune += ln + 1;
    *f_pos += ln + 1;

    return ln;
}


static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .read = fortune_read,
    .write = fortune_write,
};



static int __init init_fortune_module(void)
{
    cookie_pot = vmalloc(MAX_COOKIE_LENGTH);
    if (!cookie_pot)
    {
        printk(KERN_INFO "my_fortune: impossible to malloc cookie_pot");
        return -ENOMEM;
    }

    memset(cookie_pot, 0, MAX_COOKIE_LENGTH);

    proc_file = proc_create(PROC_FILE_NAME, 0666, NULL, &fops);

    if (!proc_file)
    {
        vfree(cookie_pot);
        printk(KERN_INFO "my_fortune: can't create proc entry");
        return -ENOMEM;
    }

    proc_mkdir("my_dir", NULL);
    proc_symlink("my_slink", NULL, "/proc/my_fortune");

    cookie_index = 0;
    next_fortune = 0;

    printk(KERN_INFO "===== Fortune module loaded! =====");

    return 0;
}

static void __exit exit_fortune_module(void)
{
    //if (proc_file)
        remove_proc_entry(PROC_FILE_NAME, NULL);

    if (cookie_pot)
        vfree(cookie_pot);

    printk(KERN_INFO "===== Fortune module unloaded! =====");
}



MODULE_LICENSE("GPL");
module_init(init_fortune_module);
module_exit(exit_fortune_module);