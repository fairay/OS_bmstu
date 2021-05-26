#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

#define		SHARED_IRQ	1

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
static struct workqueue_struct *my_wq;

void my_work_func(struct work_struct *work)
{
	printk(KERN_INFO "work: data=%lld\n", work->data.counter);
}

// DECLARE_WORK(my_work, my_work_func);	// static
struct work_struct *my_work; // dynamic

static irqreturn_t my_interrupt (int irq, void *dev_id)
{
	irq_counter++;
	printk(KERN_INFO "ISR: counter = %d\n", irq_counter);
	// queue_work(my_wq, &my_work); // static
	queue_work(my_wq, my_work); // dynamic
	return IRQ_HANDLED;
}

static int __init md_init(void) 
{
	if (request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
	{
		printk(KERN_ERR "request_irq failed\n");
		return -1;
	}

	my_wq = create_workqueue("my_queue");
	if (!my_wq)
	{
		synchronize_irq(irq);
		free_irq(irq, &my_dev_id);
		printk(KERN_ERR "create_workqueue failed\n");
		return -1;
	}

	/// dynamic
	my_work = (struct work_struct*)kmalloc(sizeof(struct work_struct), GFP_KERNEL);
	if (my_work)
	{
		INIT_WORK(my_work, my_work_func);
	}
	else 
	{
		destroy_workqueue(my_wq);
		synchronize_irq(irq);
		free_irq(irq, &my_dev_id);
	}
	///

	printk(KERN_INFO "module is loaded\n");
 	return 0;
}

static void __exit md_exit(void) 
{
	flush_workqueue(my_wq);
	destroy_workqueue(my_wq);
	
	kfree(my_work); // dynamic
	
	synchronize_irq(irq);
	free_irq(irq, &my_dev_id);

	printk(KERN_INFO "total amount of calls = %d\n", irq_counter);
	printk(KERN_INFO "module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
