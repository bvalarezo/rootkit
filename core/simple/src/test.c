#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>

#define SCTM_EXIT_POST_HOOK() my_exit()
#define SCTM_INIT_POST_HOOK() my_init()
#include "sctm.h"

/* simple linux system call hook test */

struct sctm_hook my_hook;
const char *my_name = "test";

void __exit my_exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).\n", my_name, &my_exit);
  sctm_exit();
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `my_hook_func` (%p).\n", my_name, &my_hook_func);
  return my_hook.hooked
    ? (*my_hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

int __init my_init(void) {
  int retval;
  
  printk(KERN_INFO "[%s]: In `my_init` (%p).\n", my_name, &my_init);
  my_hook = (struct sctm_hook) {
    .call = 102, /* `sys_getuid` */
    .hook = (sctm_syscall_handler_t) &my_hook_func,
    .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
  };
  retval = sctm_init();
  
  if (retval)
    return retval;
  return sctm_hook(&my_hook);
}

module_exit(my_exit)
module_init(my_init)

MODULE_LICENSE("GPL");

