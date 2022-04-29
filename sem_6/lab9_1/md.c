
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#define		SHARED_IRQ	1

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
char my_tasklet_data[]="my_tasklet_function was called";

// static
// void my_tasklet_func(unsigned long);
// DECLARE_TASKLET(my_tasklet, my_tasklet_func, (unsigned long) &my_tasklet_data);

struct tasklet_struct my_tasklet; // dynamic

void my_tasklet_func(unsigned long data)
{
	printk(KERN_INFO "tasklet RUNNING: \t\tstate: %ld \tlink count: %d\n", my_tasklet.state, my_tasklet.count.counter);
	printk("tasklet data: %s\n", (char*)data);
}

static irqreturn_t my_interrupt (int irq, void *dev_id)
{
	irq_counter++;
	printk(KERN_INFO "ISR: counter = %d\n", irq_counter);

	printk(KERN_INFO "tasklet BEFORE SCHEDULE: \tstate: %ld \tlink count: %d\n", my_tasklet.state, my_tasklet.count.counter);
	tasklet_schedule(&my_tasklet);
	printk(KERN_INFO "tasklet AFTER SCHEDULE: \t\tstate: %ld \tlink count: %d\n", my_tasklet.state, my_tasklet.count.counter);

	return IRQ_HANDLED;
}

static int __init md_init(void) 
{
	if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
		return -1;

	tasklet_init(&my_tasklet, my_tasklet_func, (unsigned long) &my_tasklet_data); // dynamic
	printk(KERN_INFO "module is loaded\n");
 	return 0;
}

static void __exit md_exit(void) 
{
	synchronize_irq(irq);
	free_irq(irq, &my_dev_id);
	tasklet_kill(&my_tasklet);

	printk(KERN_INFO "total amount of calls = %d\n", irq_counter);
	printk(KERN_INFO "module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
