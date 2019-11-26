#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>

/*
necessary definitions are included by defining
`SCTM_INCLUDE` at compile time
*/
#include "sctm.h"

/* simple linux system call hook test using `sctm` */

static struct sctm_hook my_hook;
static const char *my_name = "test";

void my_exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).\n", my_name, &my_exit);
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `my_hook_func` (%p).\n", my_name, &my_hook_func);
  return my_hook.hooked
    ? (*my_hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

int my_init(void) { 
  printk(KERN_INFO "[%s]: In `my_init` (%p).\n", my_name, &my_init);
  my_hook = (struct sctm_hook) {
    .call = 102, /* `sys_getuid` */
    .hook = (sctm_syscall_handler_t) &my_hook_func,
    .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
  };
  return sctm_hook(&my_hook);
}

