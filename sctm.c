#include "sctm.h"

/* system call table modification module */

struct sctm_hook hook = (struct sctm_hook) {
  .original = NULL
};
unsigned long _hook_call = -1;
sctm_syscall_handler_t _hook_hook = NULL;
enum sctm_unhook_mode _hook_unhook_mode = SCTM_UNHOOK_MODE_SOFT;
static struct sctm_hook *sctm__hook_wrapper_table[SCTM_TABLE_SIZE];
sctm_syscall_handler_t *table = NULL;

static void __exit sctm__exit(void) {
  /* unhook the call */

  sctm__unhook(table, &hook);
}

static int sctm__hook(sctm_syscall_handler_t *table, struct sctm_hook *hook) {
  int retval;

  if (hook == NULL)
    return -EINVAL;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  if (table == NULL)
    return -EFAULT;
  hook->original = table[hook->call];
  retval = sctm__set_syscall_handler(table, hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;
  sctm__hook_wrapper_table[hook->call] = hook; /* MUST be done last */
  return 0;
}

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
  struct sctm_hook **cur;
  int i;
  int retval;

  /* load the parameters */

  hook.call = _hook_call;
  hook.hook = _hook_hook;
  hook.unhook_mode = _hook_unhook_mode;

  if (hook.call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  /* locate the syscall table */

  retval = sctm__locate_sys_call_table(&table);

  if (retval)
    return retval;

  /* clear the hook table */

  for (cur = sctm__hook_wrapper_table, i = SCTM_TABLE_SIZE; i; i++)
    *cur = NULL;
  return sctm__hook(table, &hook);
}

static int sctm__locate_sys_call_table(sctm_syscall_handler_t **dest) {
#ifndef CONFIG_KALLSYMS
  void *max_ptr;
#endif
  if (dest == NULL)
    return -EFAULT;
#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */

  *dest = (sctm_syscall_handler_t *) kallsyms_lookup_name("sys_call_table");
#else
  /* iteratively detect for the system call table (it's cache-aligned) */

  max_ptr = (void *) ~0;

  for (*dest = 0; (void *) *dest < max_ptr && **dest != (sctm_syscall_handler_t) &SCTM_TABLE_FIRST_HANDLER; *dest += SMP_CACHE_BYTES);

  if ((void *) *dest == max_ptr)
    *dest = NULL;
#endif
  if (*dest == NULL)
    return -EFAULT;
  return 0;
}

static int sctm__set_syscall_handler(sctm_syscall_handler_t *table, unsigned long call, sctm_syscall_handler_t handler) {
  if (table == NULL)
    return -EFAULT;

  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  /////////////////////////////////////////////////////////////////
  return 0;
}

static int sctm__unhook(sctm_syscall_handler_t *table, struct sctm_hook *hook) {
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

