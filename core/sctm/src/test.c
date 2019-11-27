
/*
necessary definitions are included by defining
`SCTM_INCLUDE` at compile time
*/

#include "test.h"

/* simple linux system call hook test using `sctm` */

static struct sctm_hook my__hook = {
  .call = 102, /* `sys_getuid` */
  /* `.hook` will be set later on */
  /* `.hooked` is zeroed by the compiler */
  .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
};
static const char *my__name = "test";

void my_exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).\n", my__name, &my_exit);
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `my_hook_func` (%p).\n", my__name,
    &my_hook_func);
  return my__hook.hooked
    ? (*my__hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

int my_init(void) { 
  printk(KERN_INFO "[%s]: In `my_init` (%p).\n", my__name, &my_init);
  my__hook.hook = (sctm_syscall_handler_t) &my_hook_func;
  return sctm_hook(&my__hook);
}

