#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>

#include "sctm.h"

/* rootkit interface */

static struct sctm_hook iface__hook;

static void __exit iface__exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).\n", name, &iface__exit);
  sctm_exit();
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `iface_hook` (%p).\n", rootkit__name, &rootkit__hook_func);
  return my_hook.hooked
    ? (*my_hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

static int __init iface__init(void) {
  int retval;
  
  printk(KERN_INFO "[%s]: In `iface__init` (%p).\n", rootkit__name, &rootkit__init);
  iface__hook = (struct sctm_hook) {
    .call = 102, /* `sys_getuid` */
    .hook = (sctm_syscall_handler_t) &iface__hook_func,
    .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
  };
  retval = sctm_init();
  
  if (retval)
    return retval;
  return sctm_hook(&iface__hook);
}

module_exit(iface__exit)
module_init(iface__init)

MODULE_LICENSE("GPL");

