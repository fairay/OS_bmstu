#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/seq_file.h>

#include <linux/uaccess.h>
#include <asm/uaccess.h>


#define MAX_COOKIE_LENGTH   PAGE_SIZE
#define PROC_FILE_NAME      "my_fortune"
#define PROC_DIR_NAME       "my_dir"
#define PROC_SLINK_NAME     "my_slink"

static struct proc_dir_entry *proc_file;

static char* cookie_pot;
static int cookie_index;  
static int next_fortune;
char add_str[20] = "aaaaa\n";

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

    cookie_index += len;
    cookie_pot[cookie_index - 1] = '_';
    strcat(cookie_pot, add_str);

    cookie_index += 7;
    cookie_pot[cookie_index - 1] = 0;

    printk(KERN_INFO "proc my_writed\n");
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

    printk(KERN_INFO "proc my_readed\n");
    return ln;
}

static int simple_proc_open(struct inode *sp_inode, struct file *sp_file)
{
    printk(KERN_INFO "proc called open\n");
    return 0;
}
static int simple_proc_release(struct inode *sp_inode, struct file *sp_file)
{
    printk(KERN_INFO "proc called release\n");
    return 0;
}

static struct file_operations fops = 
{
    .owner = THIS_MODULE,
    .open = simple_proc_open,
    .release = simple_proc_release,
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

    proc_mkdir(PROC_DIR_NAME, NULL);
    proc_symlink(PROC_SLINK_NAME, NULL, "/proc/my_fortune");

    cookie_index = 0;
    next_fortune = 0;

    printk(KERN_INFO "Fortune module loaded!");

    return 0;
}

static void __exit exit_fortune_module(void)
{
    if (proc_file)
    {
        remove_proc_entry(PROC_FILE_NAME, NULL);
        remove_proc_entry(PROC_DIR_NAME, NULL);
        remove_proc_entry(PROC_SLINK_NAME, NULL);
    }

    if (cookie_pot)
        vfree(cookie_pot);

    printk(KERN_INFO "Fortune module unloaded!");
}



MODULE_LICENSE("GPL");
module_init(init_fortune_module);
module_exit(exit_fortune_module);