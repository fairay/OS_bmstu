#include "md.h" 

static int __init md_init( void ) 
{ 
    printk("MD3: module is loaded\n" ); 
    printk("MD3: md1 export string by md1_data: %s", md1_data);
	printk("MD3: md1 export string by md1_proc(): %s", md1_proc());
    printk("MD3: md1 export string by md1_noexport(): %s", md1_noexport());

    return -1;
} 

MODULE_LICENSE("GPL"); 
module_init(md_init);