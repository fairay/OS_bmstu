#include "md.h"

char* md1_data = "Привет, мир";
extern char* md1_proc(void)
{
	return md1_data;
}
static char* md1_local(void)
{
	return md1_data;
}
extern char* md1_noexport(void)
{
	return md1_data;
}

EXPORT_SYMBOL(md1_data);
EXPORT_SYMBOL(md1_proc);

static int __init md_init(void) 
{
	printk("MD1: module is loaded\n");

	// printk("MD1: md1_data: %s", md1_data);
	// printk("MD1: md1_proc(): %s", md1_proc());
	// printk("MD1: md1_noexport(): %s", md1_noexport());
	// printk("MD1: md1_local(): %s", md1_local());

 	return 0;
}


static void __exit md_exit(void) 
{
	printk("MD1: module is unloaded\n");
}

MODULE_LICENSE("GPL");
module_init(md_init);
module_exit(md_exit);
