#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* syscall table modification module */

#define MAX_CALL 0

/* hooked system call */
struct sctm_hook {
  unsigned int call;
  hook;
  int hooked;
  void *original;
};

extern int errno;
void *sctm_dummy_hook;
static struct sctm_hook hook = (struct sctm_hook) {
  .call = MAX_CALL - 1,
  .hook = sctm_dummy_hook,
  .hooked = 0
};
static void **table;

/* module cleanup */
static void __exit sctm_cleanup(void) {
  /* unhook the call */

  sctm_unhook(table, hooked);
}

/* hook the system call table */
static void sctm_hook(void **table, struct sctm_hook *hook) {
  if (hook == NULL)
    return -EINVAL;
  
  if (hook->call > MAX_CALL)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  if (table == NULL)
    return -EFAULT;
  hook->original = table[hook->call];
  table[hook->call] = hook->hook;
  hook->hooked = ~0;
  return 0;
}

/* module initialization */
static int __init sctm_init(void) {
  int retval;

  if (hook->call > MAX_CALL)
    return -EINVAL;

  /* locate the table */

  retval = sctm_locate_sys_call_table(&table);

  if (retval)
    return retval;
  return sctm_hook(&hook);
}

/* locate the system call table */
static int sctm_locate_sys_call_table(void **dest) {
  if (dest == NULL)
    return -EFAULT;
  *dest = kallsyms_lookup_name("sys_call_table");
  
  if (*dest == NULL)
    return errno ? -errno : -EFAULT;
  return 0;
}

/* unhook a system call */
static int sctm_unhook(void **table, struct sctm_hook *hook) {
  if (hook == NULL)
    return -EFAULT;
  
  if (hook->call > MAX_CALL)
    return -EINVAL;

  if (table[hook->call] == hook->hook)
    table[hook->call] = hook->original;
  hook->hooked = 0;
  return 0;
}

module_exit(sctm_exit);
module_init(sctm_init);
module_param(hook->call, unsigned int, 0x0700);
module_param(hook->hook, void *, 0x0700);

