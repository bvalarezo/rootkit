#include <linux/errno.h>
#include <linux/printk.h>

#define SCTM_EXIT_POST_HOOK() my_exit()
#define SCTM_INIT_POST_HOOK() my_init()
#include "sctm.h"

/* simple linux system call hook test */

struct sctm_hook my_hook;
const char *my_name = "test";

extern int sctm_hook(struct sctm_hook *);

void my_exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).", my_name, &my_exit);
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `my_hook_func` (%p).", my_name, &my_hook_func);
  return my_hook.hooked
    ? (*my_hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

void my_init(void) {
  printk(KERN_INFO "[%s]: In `my_init` (%p).", my_name, &my_init);
  my_hook = (struct sctm_hook) {
    .call = 102, /* `sys_getuid` */
    .hook = (sctm_syscall_handler_t) &my_hook_func,
    .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
  };
  sctm_hook(&my_hook);
}

