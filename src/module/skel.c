#include "module/module.h"

/* module */
MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("0.01");

unsigned long **SYS_CALL_TABLE;
/* module skeleton */

/* module cleanup */
static void __exit skel_exit(void) {
	printk(KERN_INFO "Peace 8)\n");

}

/* module initialization */
static int __init skel_init(void) {
	SYS_CALL_TABLE = (unsigned long**)kallsyms_lookup_name("sys_call_table");
	printk(KERN_INFO "Hello, it is Bryan!\n");
	printk(KERN_INFO "System Call Tbale is %p\n", SYS_CALL_TABLE);
	return 0;
}

module_exit(skel_exit);
module_init(skel_init);

