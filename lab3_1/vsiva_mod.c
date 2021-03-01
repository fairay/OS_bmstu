#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h> 
#include <linux/init_task.h>

static int __init md_init(void) 
{
	printk("****************** My module is loaded ******************\n");

	struct task_struct *task = &init_task;
	do
	{
		printk("TASK: %-20s %5d \t PARENT: %-20s %5d", 
			task->comm, task->pid,
			task->parent->comm, task->parent->pid);
		task = next_task(task);
	} while (task != &init_task);
	
	printk("INIT TASK: %-15s %5d \t PARENT: %-20s %5d\n\n", 
			task->comm, task->pid,
			task->parent->comm, task->parent->pid);

 	return 0;
}


static void __exit md_exit(void) 
{
	printk("****************** My module is unloaded ******************\n");
}

MODULE_LICENSE("Dual BSD/GPL");
module_init(md_init);
module_exit(md_exit);
