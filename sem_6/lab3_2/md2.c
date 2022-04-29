#include "md.h"

static int __init md_init(void) 
{
	printk("MD2: module is loaded\n");

	printk("MD2: md1 export string by md1_data: %s", md1_data);
	printk("MD2: md1 export string by md1_proc(): %s", md1_proc());
	// printk("MD2: md1 export string by md1_noexport(): %s", md1_noexport());
	// printk("MD2: md1 export string by md1_local(): %s", md1_local());

 	return 0;
}


static void __exit md_exit(void) 
{
	printk("MD2: module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
