/*
Copyright (C) 2019 Bailey Defino
<https://bdefino.github.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "sctm.h"

/* system call table modification module */

struct sctm_hook_lstacki *sctm__hooked_table[SCTM_TABLE_SIZE];
sctm_syscall_handler_t *sctm__table = NULL;

static void __exit sctm__exit(void) {
  /* call the pre-exit hook (if any) */

  SCTM_EXIT_PRE_HOOK(); /* ignore failure */

  /* unhook all hooked system calls (and ignore failure) */

  sctm_unhook_all();

  /* call the post-exit hook (if any) */

  SCTM_EXIT_POST_HOOK(); /* ignore failure */
}

int sctm_hook(struct sctm_hook *hook) {
  return sctm__hook(hook);///////////////////////////////////////////////////////////////////////////////////
}

static int sctm__hook(struct sctm_hook *hook) {
  int retval;
  
  if (hook == NULL)
    return -EINVAL;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  if (sctm__table == NULL)
    return -EFAULT;

  /* hook */

  hook->original = sctm__table[hook->call];
  retval = sctm__set_syscall_handler(hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;
  return 0;
}

asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0,
    unsigned long arg1, unsigned long arg2, unsigned long arg3,
    unsigned long arg4) {
  struct sctm_hook_lstack *cur;
  struct sctm_hook *hook;
  struct sctm_hook_lstacki *iter;
  int retval;

  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  /* need initialization */

  if (sctm__table == NULL)
    return -EFAULT;

  /* locate hook */

  iter = sctm__hooked_table[call];

  if (iter == NULL
      || iter->cur == NULL
      || iter->cur->hook == NULL)
    /* not hooked */

    return -EFAULT;

  /* save state */

  cur = iter->cur;

  /*
  delegate under a new state
  ((indirect) recursion should terminate automatically)
  */

  hook = cur->hook;
  iter->cur = cur->next;
  retval = -EFAULT;

  if (hook->hooked) {
    if (hook->hook != NULL)
      retval = (*hook->hook)(call, arg0, arg1, arg2, arg3, arg4);
  } else if (hook->original != NULL)
    retval = (*hook->original)(call, arg0, arg1, arg2, arg3, arg4);
  
  /* restore state */

  iter->cur = cur;
  return retval;
};

static int __init sctm__init(void) {
  int i;
  int retval;

  /* call the pre-initialization hook (if any) */

  retval = SCTM_INIT_PRE_HOOK();

  if (retval)
    return retval;

  /* wipe the table */

  for (i = 0; i < SCTM_TABLE_SIZE; i++)
    sctm__hooked_table[i] = NULL;

  /* locate the syscall table */

  retval = sctm__locate_sys_call_table();

  if (retval)
    return retval;

  /* call the post-initialization hook (if any) */

  return SCTM_INIT_POST_HOOK();
}

static int sctm__locate_sys_call_table(void) {
#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */

  sctm__table = (sctm_syscall_handler_t *)
    kallsyms_lookup_name("sys_call_table");
#else
  /* incrementing hash table for perceived addresses */
  struct {
    unsigned int size;
    struct {
      int count;
      void *p;
    } table[SCTM_TABLE_SIZE];
  } addresses;
  unsigned int i;
  void *max_ptr;

  /*
  search for the system call table

  unfortunately, the locations of defined functions
  aren't available for the following reasons:
  1. the symbol table is inaccessible
  and
  2. system call addresses are defined at runtime,
    so we can't do a serial search for a specific value

  however, these properties of the system call table provide a neat
  (albeit more expensive) workaround:
  1. the table is cache-aligned
  2. the size is known
  3. a rough upper limit on the number of entries is known
  and
  4. undefined entries have the same value

  we can use this information to determine the probability that a memory
  address is the start of the system call table
  */

  for (i = 0; i < SCTM_TABLE_SIZE; i++)
    addresses.table[i].count = 0;
    /* no need to set `addresses.table[i].p` */
  max_ptr = (void *) ~0;

  for (sctm__table = 0; (void *) sctm__table < max_ptr
      && *sctm__table != SCTM_TABLE_FIRST_ENTRY;
      sctm__table += SMP_CACHE_BYTES);

  if ((void *) sctm__table >= max_ptr)
    sctm__table = NULL;
#endif
  if (sctm__table == NULL
      || *sctm__table != SCTM_TABLE_FIRST_ENTRY)
    return -EFAULT;
  return 0;
}

static int sctm__return_0(void) {
  return 0;
}

static int sctm__set_syscall_handler(unsigned long call,
    sctm_syscall_handler_t handler) {
  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  /////////////////////////////////////////////////////////////////
  return 0;
}

int sctm_unhook(struct sctm_hook *hook) {
  return 0;//////////////////////////////////////////////////////
}

//////////////////////////////////////////////////////rewrite and add compatibility for the linked hook table
static int sctm__unhook(struct sctm_hook *hook) {
  int retval;

  if (hook == NULL)
    return -EFAULT;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (sctm__table[hook->call] == hook->hook
      && hook->unhook_method == SCTM_UNHOOK_METHOD_REPLACE) {
    /* replace with the original entry */

    retval = sctm__set_syscall_handler(hook->call, hook->original);

    if (retval)
      return retval;
  }
  hook->hooked = 0;
  return 0;
}

int sctm_unhook_all(void) {
  return sctm__unhook(NULL);//////////////////////////////////////////////////////
}

