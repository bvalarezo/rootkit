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

/* locate the system call table */
static int sctm__locate_sys_call_table(struct sctm *sctm);

/* set a system call handler */
static int sctm__set_syscall_handler(struct sctm *sctm,
  const unsigned long call, const sctm_syscall_handler_t handler);

int sctm_cleanup(struct sctm *sctm) {
  int retval;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  /* remove all hooks */
  
  retval = sctm_unhook_all(sctm);
  
  if (retval)
    return retval;
  sctm->table = NULL;
  return 0;
}

int sctm_hook(struct sctm *sctm, struct sctm_hook *hook) {
  int retval;
  
  if (hook == NULL)
    return -EFAULT;
  
  if (IS_ERR(hook))
    return -EINVAL;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  
  if (!IS_ERR_OR_NULL(sctm->hook_registry[hook->call]))
    /* this call's already been hooked */
    
    return -EINVAL;

  if (hook->hooked)
    return 0;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  if (sctm->table == NULL)
    return -EINVAL;
  
  if (IS_ERR(sctm->table))
    return -EINVAL;
  
  /* hook */

  hook->original = sctm->table[hook->call];
  retval = sctm__set_syscall_handler(sctm, hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;

  /* register */

  sctm->hook_registry[hook->call] = hook;
  return 0;
}

int sctm_init(struct sctm *dest) {
  int call;
  
  if (dest == NULL)
    return -EFAULT;
  
  if (IS_ERR(dest))
    return -EINVAL;
  
  /* wipe the hook registry */

  for (call = 0; call < SCTM_TABLE_SIZE; call++)
    dest->hook_registry[call] = NULL;

  /* locate the syscall table */

  return sctm__locate_sys_call_table(dest);
}

static int sctm__locate_sys_call_table(struct sctm *sctm) {
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  /* (re)locate the system call table */
  
  sctm->table = (sctm_syscall_handler_t *)
    kallsyms_lookup_name("sys_call_table");
  return IS_ERR_OR_NULL(sctm->table) ? -EFAULT : 0;
}

static int sctm__set_syscall_handler(struct sctm *sctm,
    const unsigned long call, const sctm_syscall_handler_t handler) {
  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  
  if (handler == NULL)
    return -EFAULT;
  
  if (IS_ERR(handler))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  /* need initialization */
  
  if (sctm->table == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm->table))
    return -EINVAL;
  
  /* enable writing */
  
  write_cr0(read_cr0() & ~X86_CR0_WP);
  
  /* overwrite */
  
  sctm->table[call] = handler;
  
  /* disable writing */
  
  write_cr0(read_cr0() | X86_CR0_WP);
  return 0;
}

int sctm_unhook(struct sctm *sctm, struct sctm_hook *hook) {
  int retval;
  
  if (IS_ERR_OR_NULL(hook))
    return -EFAULT;

  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  
  if (!hook->hooked)
    return 0;
  
  if (IS_ERR_OR_NULL(hook->original))
    return -EFAULT;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  /* unhook */
  
  hook->hooked = 0;
  retval = sctm__set_syscall_handler(sctm, hook->call, hook->original);
  
  if (retval)
    /* failed: don't deregister */
    
    return retval;
  
  /* deregister */
  
  sctm->hook_registry[hook->call] = NULL;
  return 0;
}

int sctm_unhook_all(struct sctm *sctm) {
  unsigned long call;
  int retval;
  int _retval;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  
  if (sctm->table == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm->table))
    return -EINVAL;
  
  /* unhook/deregister as many hooks as possible */
  
  retval = 0;
  
  for (call = 0; call < SCTM_TABLE_SIZE; call++) {
    if (IS_ERR_OR_NULL(sctm->hook_registry[call]))
      continue;
    _retval = sctm_unhook(sctm, sctm->hook_registry[call]);

    if (_retval
        && !retval)
      /* persist only the first error */
      
      retval = _retval;
  }
  return 0;
}

