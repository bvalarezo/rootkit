#include "sctm.h"

/* system call table modification module */

struct sctm_hook hook = (struct sctm_hook) {
  .hooked = 0,
  .original = NULL
};
unsigned int _hook_call = 0;
sctm_syscall_handler_t _hook_hook = NULL;
sctm_syscall_handler_t *table = NULL;

static void __exit sctm_exit(void) {
  /* unhook the call */

  sctm_unhook(table, &hook);
}

static int sctm_hook(sctm_syscall_handler_t *table, struct sctm_hook *hook) {
  if (hook == NULL)
    return -EINVAL;
  
  if (hook->call > SCTM_TABLE_SIZE)
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

static int __init sctm_init(void) {
  int retval;

  /* load the parameters */

  hook.call = _hook_call;
  hook.hook = _hook_hook;

  if (hook.call > SCTM_TABLE_SIZE)
    return -EINVAL;

  /* locate the table */

  retval = sctm_locate_sys_call_table(&table);

  if (retval)
    return retval;
  return sctm_hook(table, &hook);
}

static int sctm_locate_sys_call_table(sctm_syscall_handler_t **dest) {
#ifndef CONFIG_KALLSYMS
  void *max_ptr;
#endif
  if (dest == NULL)
    return -EFAULT;
#if CONFIG_KALLSYMS
  *dest = (sctm_syscall_handler_t *) kallsyms_lookup_name("sys_call_table");
#else
  /* iteratively detect for the system call table (it's cache-aligned) */

  max_ptr = (void *) ~0;

  for (*dest = 0; (void *) *dest < max_ptr && **dest != (sctm_syscall_handler_t) &sys_read; *dest += SMP_CACHE_BYTES);

  if ((void *) *dest == max_ptr)
    *dest = NULL;
#endif
  if (*dest == NULL)
    return -EFAULT;
  return 0;
}

static int sctm_unhook(sctm_syscall_handler_t *table, struct sctm_hook *hook) {
  if (hook == NULL)
    return -EFAULT;
  
  if (hook->call > SCTM_TABLE_SIZE)
    return -EINVAL;

  if (table[hook->call] == hook->hook)
    table[hook->call] = hook->original;
  hook->hooked = 0;
  return 0;
}

