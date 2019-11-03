#include "sctm.h"

/* system call table modification module */

struct sctm_hook_lstack *hooked_table[SCTM_TABLE_SIZE] = {
  [0 ... SCTM_TABLE_SIZE] NULL
};

sctm_syscall_handler_t *table = NULL;

static void __exit sctm__exit(void) {
  /* unhook all hooked system calls (and ignore failure) */

  sctm__unhook_all(table, hook_stack);

  /* call the exit hook (if any) */

  return SCTM__EXIT_HOOK();
}

int sctm_hook(struct sctm_hook *hook) {
  return 0;//////////////////////////////////////////////
}

static int sctm__hook(struct sctm_hook *hook) {
  struct sctm_hook_lstack *node;
  int retval;
  
  if (hook == NULL)
    return -EINVAL;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  if (table == NULL)
    return -EFAULT;

  /* hook */

  hook->original = table[hook->call];
  retval = sctm__set_syscall_handler(table, hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;
  return 0;
}

/////////////////////////////////////possibly rewrite, but add differentiation for multiple hooks on the same syscall (or not?)
asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4) {
  struct sctm_hook *hook;
  int retval;

  if (call > SCTM_TABLE_SIZE)
    return -EINVAL;

  /* need syscall table */

  if (table == NULL) {
    retval = sctm__locate_sys_call_table(&table);

    if (retval)
      return retval;
  }

  /* find the hook */

  hook = sctm__hook_wrapper_table[call];

  if (hook == NULL)
    return -EFAULT;

  if (!hook->hooked
      || hook->hook == NULL) {
    /* original behavior */

    if (hook->original == NULL)
      return -EFAULT;
    return (*hook->original)(call, arg0, arg1, arg2, arg3, arg4);
  }

  /* hook */

  return (*hook->hook)(call, arg0, arg1, arg2, arg3, arg4);
};

static int __init sctm__init(void) {
  int retval;

  /* call the initialization hook (if any) */

  retval = SCTM__INIT_HOOK();

  if (retval)
    return retval;

  /* locate the syscall table */

  retval = sctm__locate_sys_call_table(&table);

  if (retval)
    return retval;
}

static int sctm__locate_sys_call_table(void) {
#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */

  table = (sctm_syscall_handler_t *) kallsyms_lookup_name("sys_call_table");
#else
  void *max_ptr;
  /* iteratively detect for the system call table (it's cache-aligned) */

  max_ptr = (void *) ~0;

  for (table = 0; (void *) table < max_ptr && **dest != (sctm_syscall_handler_t) &SCTM_TABLE_FIRST_HANDLER; table += SMP_CACHE_BYTES);

  if ((void *) table == max_ptr)
    table = NULL;
#endif
  if (table == NULL)
    return -EFAULT;
  return 0;
}

static int sctm__set_syscall_handler(unsigned long call, sctm_syscall_handler_t handler) {
  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  /////////////////////////////////////////////////////////////////
  return 0;
}

int sctm_unhook(struct sctm_hook *hook) {
  //////////////////////////////////////////////////////
}

////////////////////////////////////rewrite and add compatibility for the linked hook table
static int sctm__unhook(struct sctm_hook *hook) {
  int retval;

  if (hook == NULL)
    return -EFAULT;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (table[hook->call] == hook->hook
      && hook->unhook_mode == SCTM_UNHOOK_MODE_HARD) {
    /* replace with the original entry */

    retval = sctm__set_syscall_handler(table, hook->call, hook->original);

    if (retval)
      return retval;
  }
  hook->hooked = 0;
  return 0;
}

