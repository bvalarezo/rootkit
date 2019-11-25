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
  double dev; /* individual deviation */
  unsigned double mean;
  unsigned int modei; /* index of the mode */
  sctm_syscall_handler_t poisoned; /* poisoned pointer */
  /* pointer within the possible system call table */
  sctm_syscall_handler_t *ptr;
  int retval;
  unsigned double sd; /* standard deviation */
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
  3. a rough upper limit on the number of entries is known
  and
  4. undefined entries have the same value

  we can use this information to determine the probability that a memory
  address is the start of the system call table
  */

  /* poison our pointer */

  for (ai = 0; ai < sizeof(sctm_syscall_handler_t); ai++)
    ((char *) poisoned)[ai] = (char) PAGE_POISON;

  /* search for the system call table */

  for (table = 0; table <= max_ptr; table += SMP_CACHE_SIZE) {
    /* clear histogram */

    for (addrs.count = ai = 0; ai < SCTM_TABLE_SIZE; ai++)
      addrs.hist[ai].count = 0;

    /* populate histogram */

    for (ti = 0, ptr = table; ti < SCTM_TABLE_SIZE; ti++) {
      if (ptr >= SCTM_MEM_MAX_ADDRESS)
        /* exhausted address space */

        return -EFAULT;
      
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

    for (ai = mean = modei = 0; ai < SCTM_TABLE_SIZE; ai++) {
      if (addrs.hist[ai].count > addrs.hist[modei].count)
        modei = ai;
      mean += addrs.hist[ai].count;
    }
    mean /= addrs.count;

    /* calculate the standard deviation */

    for (ai = sd = 0; ai < SCTM_TABLE_SIZE; ai++) {
      dev = addrs.hist[ai] - mean;
      sd += dev >= 0 ? dev : -dev; /* `sd` must always be positive */
    }
    sd /= addrs.count;

    /*
    classify the histogram's mode as either
    a repetition of `PAGE_POISON` (=> not the system call table)
    or `&sys_ni_syscall` (=> `table` could be the system call table)
    */

    if (addrs.hist[modei].count - mean > sd) {
      /* the mode's a definite outlier, as in the system call table */

      if (addrs.hist[modei].handler == poisoned)
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

