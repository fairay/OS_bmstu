#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h> 
#include <linux/init_task.h>

static int __init md_init(void) 
{
	printk("MD: module is loaded\n");

	struct task_struct *task = &init_task;
	do
	{
		printk("TASK: %-20s %5d \t PARENT: %-20s %5d", 
			task->comm, task->pid,
			task->parent->comm, task->parent->pid);
		task = next_task(task);
	} while (task != &init_task);
	
	printk("CUR TASK: %-16s %5d",
			current->comm, current->pid);

 	return 0;
}


static void __exit md_exit(void) 
{
	printk("MD: module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
