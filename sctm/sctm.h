#ifndef SCTM_H
#define SCTM_H

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

/* system call table modification module */

#ifndef SCTM__EXIT_HOOK
#define SCTM__EXIT_HOOK() 0 /* module exit hook */
#endif
#ifndef SCTM__INIT_HOOK
#define SCTM__INIT_HOOK() 0 /* module initialization hook */
#endif
#ifndef SCTM_TABLE_FIRST_HANDLER
#define SCTM_TABLE_FIRST_HANDLER sys_read /* x86-64 */
#endif
#ifndef SCTM_TABLE_SIZE
#define SCTM_TABLE_SIZE 547 /* x86-64 */
#endif

/* convenient type alias */
typedef asmlinkage long (*sctm_syscall_handler_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

/* unhook methods */
enum sctm_unhook_method {
  SCTM_UNHOOK_METHOD_DISABLE, /* disable the hook by instead calling the original */
  SCTM_UNHOOK_METHOD_REPLACE /* replace the system call table entry with its original value */
};

/* hooked system call */
struct sctm_hook {
  unsigned long call; /* system call number (index within system call table) */
  sctm_syscall_handler_t hook; /* hook function */
  int hooked; /* flag for whether the hook should provide new functionality */
  enum sctm_unhook_method unhook_method; /* how to unhook the system call */ 
  sctm_syscall_handler_t original;
};

/* linked stack of hooked system calls */
struct sctm_hook_lstack {
  struct sctm_hook *hook;
  struct sctm_hook_stack *next;
};

struct sctm_hook_lstack *hooked_table[SCTM_TABLE_SIZE]; /* quick lookup for hooked system calls */
extern sctm_syscall_handler_t SCTM_TABLE_FIRST_HANDLER;
sctm_syscall_handler_t *table;

/* module cleanup */
static void __exit sctm__exit(void);

/* hook and register a system call */
int sctm_hook(struct sctm_hook *hook);

/* hook a system call */
static int sctm__hook(struct sctm_hook *hook);

/* call the hook and/or the original */
asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);

/* module initialization */
static int __init sctm__init(void);

/* locate the system call table */
static int sctm__locate_sys_call_table(void);

/* set a system call handler */
static int sctm__set_syscall_handler(unsigned long call, sctm_syscall_handler_t handler);

/* unhook and possibly deregister a hooked system call */
int sctm_unhook(struct sctm_hook *hook);

/* unhook a hooked system call */
static int sctm__unhook(struct sctm_hook *hook);

/* unhook and deregister all hooked system calls */
int sctm_unhook_all(void);

MODULE_LICENSE("GPL");

module_exit(sctm__exit);
module_init(sctm__init);

#endif

