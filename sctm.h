#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/syscalls.h>

/* system call table modification module */

/* #define SCTM_KALLSYMS_LOOKUP_NAME */
#define SCTM_TABLE_SIZE __NR_syscall_max

/* convenient type alias */
typedef asmlinkage long (*sctm_syscall_handler_t)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

/* hooked system call */
struct sctm_hook {
  unsigned int call;
  sctm_syscall_handler_t hook;
  int hooked;
  sctm_syscall_handler_t original;
};

extern int errno;
struct sctm_hook hook;
unsigned int _hook_call;
sctm_syscall_handler_t _hook_hook;
sctm_syscall_handler_t *table;

extern sctm_syscall_handler_t sys_read;

/* module cleanup */
static void __exit sctm_exit(void);

/* hook the system call table */
static int sctm_hook(sctm_syscall_handler_t *table, struct sctm_hook *hook);

/* module initialization */
static int __init sctm_init(void);

/* locate the system call table */
static int sctm_locate_sys_call_table(sctm_syscall_handler_t **dest);

/* unhook a system call */
static int sctm_unhook(sctm_syscall_handler_t *table, struct sctm_hook *hook);

module_exit(sctm_exit);
module_init(sctm_init);

/*module_param(_hook_call, int, 0x0700);
module_param(_hook_hook, sctm_syscall_handler_t, 0x0700);*/

