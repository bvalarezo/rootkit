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

#ifndef SCTM_TABLE_FIRST_HANDLER
#define SCTM_TABLE_FIRST_HANDLER sys_read /* x86-64 */
#endif
#ifndef SCTM_TABLE_SIZE
#define SCTM_TABLE_SIZE 547 /* x86-64 */
#endif

/* convenient type alias */
typedef asmlinkage long (*sctm_syscall_handler_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

/* unhook modes */
enum sctm_unhook_mode {
  SCTM_UNHOOK_MODE_HARD, /* replace with the original handler and unset the hooked flag */
  SCTM_UNHOOK_MODE_SOFT /* unset the hooked flag and assume the handler will only provide original functionality */
};

/* hooked system call */
struct sctm_hook {
  unsigned long call;
  sctm_syscall_handler_t hook;
  int hooked; /* flag for whether the hook should provide new functionality */
  enum sctm_unhook_mode unhook_mode;
  sctm_syscall_handler_t original;
};

//////////////////////////////////////////////wrapper table is limited to 1 hook per call:
///////////////////////////////////////////////this is the workaround basei (not yet integrated)
/* linked list of hooks */
struct sctm_hook_list {
  struct sctm_hook *hook;
  struct sctm_hook_list *next;
};

struct sctm_hook hook;
unsigned long _hook_call;
sctm_syscall_handler_t _hook_hook;
enum sctm_unhook_mode _hook_unhook_mode;
static struct sctm_hook_list *sctm__hook_wrapper_table[SCTM_TABLE_SIZE]; /* workaround for not being able to pass a hook to sctm__hook_wrapper */
extern sctm_syscall_handler_t SCTM_TABLE_FIRST_HANDLER;
sctm_syscall_handler_t *table;

/* module cleanup */
static void __exit sctm__exit(void);

/* hook the system call table */
static int sctm__hook(sctm_syscall_handler_t *table, struct sctm_hook *hook);

/* call the hook and/or the original */
asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4);

/* module initialization */
static int __init sctm__init(void);

/* locate the system call table */
static int sctm__locate_sys_call_table(sctm_syscall_handler_t **dest);

/* set a system call handler */
static int sctm__set_syscall(sctm_syscall_handler_t *table, long call, sctm_syscall_handler_t handler);

/* unhook a system call */
static int sctm__unhook(sctm_syscall_handler_t *table, struct sctm_hook *hook);

MODULE_LICENSE("GPL");

module_exit(sctm__exit);
module_init(sctm__init);

/*module_param(_hook_call, int, 0x0700);
module_param(_hook_hook, sctm_syscall_handler_t, 0x0700);*/

#endif

