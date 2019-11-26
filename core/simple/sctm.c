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

struct sctm_hook *sctm__hook_registry[SCTM_TABLE_SIZE];
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
  int retval;
  
  if (hook == NULL)
    return -EFAULT;
  
  if (IS_ERR(hook))
    return -EINVAL;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  /* need initialization */

  if (sctm__table == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm__table))
    return -EINVAL;
  
  if (!IS_ERR_OR_NULL(sctm__hook_registry[hook->call]))
    /* this call's already been hooked */
    
    return -EINVAL;
  
  /* hook */

  hook->original = sctm__table[hook->call];
  retval = sctm__set_syscall_handler(hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;

  /* register */

  sctm__hook_registry[hook->call] = hook;
  return 0;
}

static int __init sctm__init(void) {
  int call;
  int retval;

  /* call the pre-initialization hook (if any) */

  retval = SCTM_INIT_PRE_HOOK();

  if (retval)
    return retval;

  /* wipe the hook registry */

  for (call = 0; call < SCTM_TABLE_SIZE; call++)
    sctm__hook_registry[call] = NULL;

  /* locate the syscall table */

  retval = sctm__locate_sys_call_table();

  if (retval)
    return retval;

  /* call the post-initialization hook (if any) */

  return SCTM_INIT_POST_HOOK();
}

static int sctm__locate_sys_call_table(void) {
  if (sctm__table != NULL)
    /* the table seems to have already been found */

    return 0;
  sctm__table = (sctm_syscall_handler_t *)
    kallsyms_lookup_name("sys_call_table");
  return IS_ERR_OR_NULL(sctm__table) ? -EFAULT : 0;
}

static inline int sctm__return_0(void) {
  return 0;
}

static int sctm__set_syscall_handler(const unsigned long call,
    const sctm_syscall_handler_t handler) {
  unsigned long page;
  int retval;
  
  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  
  if (handler == NULL)
    return -EFAULT;
  
  if (IS_ERR(handler))
    return -EINVAL;
  
  /* need initialization */
  
  if (sctm__table == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm__table))
    return -EINVAL;
  
  /* need page */
  
  page = (unsigned long) (PAGE_ALIGN((unsigned long) sctm__table) - PAGE_SIZE);
  
  /* enable writing */
  
  retval = set_memory_rw(page, 1);
  
  /* overwrite */
  
  sctm__table[call] = handler;
  
  /* disable writing */
  
  retval = set_memory_ro(page, 1);
  return retval;
}

int sctm_unhook(struct sctm_hook *hook) {
  int retval;
  
  if (IS_ERR_OR_NULL(hook))
    return -EFAULT;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  
  if (!hook->hooked)
    return 0;
  
  if (IS_ERR_OR_NULL(hook->original))
    return -EFAULT;
  
  /* unhook */
  
  hook->hooked = 0;
  retval = sctm__set_syscall_handler(hook->call, hook->original);
  
  if (retval)
    /* failed: don't deregister */
    
    return retval;
  
  /* deregister */
  
  sctm__hook_registry[hook->call] = NULL;
  return 0;
}

int sctm_unhook_all(void) {
  unsigned long call;
  int retval;
  int _retval;

  /* unhook/deregister as many hooks as possible */
  
  retval = 0;
  
  for (call = 0; call < SCTM_TABLE_SIZE; call++) {
    if (IS_ERR_OR_NULL(sctm__hook_registry[call]))
      continue;
    _retval = sctm_unhook(sctm__hook_registry[call]);
    
    if (_retval
        && !retval)
      /* persist only the first error */
      
      retval = _retval;
  }
  return 0;
}

