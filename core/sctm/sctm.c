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

struct sctm_hook_lstacki *sctm__hook_registry[SCTM_TABLE_SIZE];
sctm_syscall_handler_t *sctm__table = NULL;

static void __exit sctm__exit(void) {
  /* call the pre-exit hook (if any) */

  SCTM_EXIT_PRE_HOOK(); /* ignore failure */

  /* unhook all hooked system calls (and ignore failure) */

  sctm_unhook_all();

  /* call the post-exit hook (if any) */

  SCTM_EXIT_POST_HOOK(); /* ignore failure */
}

int sctm_hook(struct sctm_hook *hook) {//////////////////////////////////////better error checking
  struct sctm_hook_lstack *entry;
  struct sctm_hook_lstacki *iter;
  int retval;

  if (IS_ERR_OR_NULL(hook))
    return -EFAULT;
  
  if (hook->call >= SCTM_TABLE_SIZE)
    return -EINVAL;

  if (hook->hooked)
    return 0;

  /* need initialization */

  if (IS_ERR_OR_NULL(sctm__table))
    return -EFAULT;

  /* need registry iterator */

  iter = sctm__hook_registry[hook->call];

  if (iter->syscall_in_progress)
    /* hooking will upset state restoration */

    return -EINVAL;

  /* hook */

  hook->original = sctm__table[hook->call];
  retval = sctm__set_syscall_handler(hook->call, hook->hook);

  if (retval)
    return retval;
  hook->hooked = ~0;

  /* need registry entry */

  entry = (struct sctm_hook_lstack *)
    kmalloc(sizeof(struct sctm_hook_lstack), GFP_KERNEL);

  if (IS_ERR_OR_NULL(node)) {
    /* unhook */

    hook->hooked = 0;
    sctm__set_syscall_handler(hook->call, hook->original); /* ignore failure */
    return node;
  }
  entry->hook = hook;
  entry->next = iter->stack;

  /* register */

  if (iter->cur == iter->stack)
    /* update the head */

    iter->cur = entry;
  iter->stack = entry;
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

  if (IS_ERR_OR_NULL(sctm__table))
    return -EFAULT;

  /* locate hook */

  iter = sctm__hook_registry[call];

  if (IS_ERR_OR_NULL(iter))
    return IS_ERR(iter) ? -ERR
      || IS_ERR_OR_NULL(iter->cur)
      || IS_ERR_OR_NULL(iter->cur->hook))
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

  /* wipe the hook registry */

  for (i = 0; i < SCTM_TABLE_SIZE; i++)
    sctm__hook_registry[i] = (struct sctm_hook_lstacki) {
      .cur = NULL,
      .stack = NULL,
      .syscall_in_progress = 0
    };

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
#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */

  sctm__table = (sctm_syscall_handler_t *)
    kallsyms_lookup_name("sys_call_table");
#else
  size_t addr_hash;
  struct sctm_hist addrs; /* histogram of addresses */
  unsigned int ai;
  unsigned int ai_final;
  unsigned double count_dev; /* deviation of a single count */
  unsigned double count_mean;
  unsigned int count_modei; /* mode index */
  unsigned double count_sd; /* standard deviation */
  sctm_syscall_handler_t poisoned; /* poisoned pointer */
  /* pointer within the possible system call table */
  sctm_syscall_handler_t *ptr;
  int retval;
  sctm_syscall_handler_t *table; /* possible system call table */
  unsigned int ti;

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
  3. there's a rough upper limit on the number of entries
  and
  4. undefined entries have the same value

  we can use this information to determine the probability that a memory
  address is the start of the system call table
  */

  /* poison our pointer */

  for (ai = 0; ai < sizeof(sctm_syscall_handler_t); ai++)
    ((char *) poisoned)[ai] = (char) PAGE_POISON;

  /* search for the system call table */

  for (table = SCTM_MEM_MIN_ADDRESS;
      SCTM_MEM_MAX_ADDRESS - SMP_CACHE_SIZE >= 0;
      table += SMP_CACHE_SIZE) { /* prevent address space exhaustion early */
    /* clear histogram */

    for (addrs.count = ai = 0; ai < SCTM_TABLE_SIZE; ai++)
      addrs.hist[ai].count = 0;

    /* populate histogram */

    for (ti = 0, ptr = table; ti < SCTM_TABLE_SIZE; ptr++, ti++) {
      /*
      add the possible address to the histogram
      
      this won't fail, because the table is a sufficient size
      */

      ai = sctm__sum_mod_hash(&addr_hash, ptr,
        sizeof(sctm_syscall_handler_t), SCTM_TABLE_SIZE);

      for (ai_final = ai > 0 ? ai - 1 : SCTM_TABLE_SIZE - 1;
          ai != ai_final; ai++) {
        if (!addrs.hist[ai].count
            || addrs.hist[ai].value == *ptr) {
          addrs.count++;
          addrs.hist[count]++;
          addrs.hist[ai].value = *ptr;
          break;
        }
      }
    }

    /* calculate the mean and mode */

    count_mean = 0.0;
    count_modei = 0;

    for (ai = 0; ai < SCTM_TABLE_SIZE; ai++) {
      if (addrs.hist[ai].count > addrs.hist[count_modei].count)
        count_modei = ai;
      count_mean += addrs.hist[ai].count;
    }
    count_mean /= addrs.count;

    /* calculate the standard deviation */

    count_sd = 0.0;

    for (ai = 0; ai < SCTM_TABLE_SIZE; ai++) {
      /* `count_sd` must be positive */

      count_dev = addrs.hist[ai] - count_mean;
      count_sd += count_dev >= 0 ? count_dev : -count_dev;
    }
    count_sd /= addrs.count;

    /*
    classify the histogram's mode as either
    a repetition of `PAGE_POISON` (=> not the system call table)
    or `&sys_ni_syscall` (=> `table` could be the system call table)
    */

    if (addrs.hist[count_modei].count - mean >
        SCTM_TABLE_CLASSIFIER_SYS_NI_SYSCALL_COUNT_DEVIATIONS_MIN
          * count_sd) {
      /* the mode's a definite outlier, as in the system call table */

      if (addrs.hist[count_modei].handler == poisoned)
        /*
        it's filled with the `PAGE_POISON` value,
        so likely not the system call table
        */
        
        continue;
      sctm__table = table;
      break;
    }
  }
#endif
  return sctm__table != NULL ? 0 : -EFAULT;
}

static inline int sctm__return_0(void) {
  return 0;
}

static int sctm__set_syscall_handler(const unsigned long call,
    const sctm_syscall_handler_t handler) {
  if (call >= SCTM_TABLE_SIZE)
    return -EINVAL;
  /////////////////////////////////////////////////////////////////
  return 0;
}
#ifndef CONFIG_KALLSYMS
static inline int sctm__sum_mod_hash(size_t *dest, const unsigned char *buf,
    size_t count, const unsigned short mod) {
  if (dest == NULL)
    return -EFAULT;
  *dest = 0; /* prep failure */

  if (buf == NULL)
    return -EFAULT;

  if (!mod)
    return -EINVAL;

  for (; count; buf++, count--)
    *dest = (*buf + *dest) % mod; /* trade speed for overflow protection */
  return 0;
}
#endif
//////////////////////////////////////////////////////rewrite and add compatibility for the linked hook table
int sctm_unhook(struct sctm_hook *hook) {
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
  unsigned long call;
  struct sctm_hook_lstacki *iter;
  int retval;
  int _retval;

  /* unhook as many hooks as possible */

  for (call = retval = 0; call < SCTM_TABLE_SIZE; call++) {
    iter = sctm__hook_registry[call];

    while (!IS_ERR_OR_NULL(iter->stack)) {
      iter->cur = iter->stack;
      iter->stack = iter->stack->next;

      /* unhook */

      _retval = sctm_unhook(cur->hook);

      if (_retval
          && !retval)
        retval = _retval;
      kfree(iter->cur);
    }
    
    if (!IS_ERR_OR_NULL(iter->cur))
      kfree(iter->cur);
  }
  return 0;
}

