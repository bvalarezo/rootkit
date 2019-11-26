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
#ifndef SCTM_H
#define SCTM_H

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poison.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

/* system call table modification module */

#ifndef CONFIG_KALLSYMS

/* minimum 32-bit kernel-space virtual address */
#ifndef SCTM__32_MEM_MIN_ADDRESS
#define SCTM__32_MEM_MIN_ADDRESS ((void *) 0xC0000000)
#endif
/* minimum 64-bit kernel-space virtual address */
#ifndef SCTM__64_MEM_MIN_ADDRESS
#define SCTM__64_MEM_MIN_ADDRESS ((void *) 0xFFFF800000000000)
#endif

#endif

/* module post-exit hook */
#ifndef SCTM_EXIT_POST_HOOK
#define SCTM_EXIT_POST_HOOK() sctm__return_0()
#endif
/* module pre-exit hook */
#ifndef SCTM_EXIT_PRE_HOOK
#define SCTM_EXIT_PRE_HOOK() sctm__return_0()
#endif
/* module post-initialization hook (return success) */
#ifndef SCTM_INIT_POST_HOOK
#define SCTM_INIT_POST_HOOK() sctm__return_0()
#endif
/* module pre-initialization hook (return success) */
#ifndef SCTM_INIT__PRE_HOOK
#define SCTM_INIT_PRE_HOOK() sctm__return_0()
#endif

#ifndef CONFIG_KALLSYMS

/* maximum kernel-space virtual memory address */
#ifndef SCTM_MEM_MAX_ADDRESS
#define SCTM_MEM_MAX_ADDRESS (~((void *) 0))
#endif
/* minimum kernel-space virtual memory address */
#ifndef SCTM_MEM_MIN_ADDRESS
#define SCTM_MEM_MIN_ADDRESS SCTM__64_MEM_MIN_ADDRESS
#endif
/*
the maximum number of syscalls
(obtained by liberally consolidating "include/linux/syscalls.h" by hand
and the `grep`ping for "^asmlinkage")
*/
#ifndef SCTM_TABLE_CLASSIFIER_MAX_SYSCALLS
#define SCTM_TABLE_CLASSIFIER_MAX_SYSCALLS 425
#endif
/*
minimum number of standard deviations between a possible `sys_ni_syscall`
address and other addresses in the system call table
*/
#ifndef SCTM_TABLE_CLASSIFIER_SYS_NI_SYSCALL_COUNT_DEVIATIONS_MIN
#define SCTM_TABLE_CLASSIFIER_SYS_NI_SYSCALL_COUNT_DEVIATIONS_MIN 2
#endif

#endif

/* system call table size */
#ifndef SCTM_TABLE_SIZE
#define SCTM_TABLE_SIZE SCTM__X86_64_TABLE_SIZE
#endif
/* system call table size on x86_64 */
#ifndef SCTM__X86_64_TABLE_SIZE
#define SCTM__X86_64_TABLE_SIZE 547
#endif

/* convenient type alias */
typedef asmlinkage long (*sctm_syscall_handler_t)(unsigned long,
  unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);

/* unhook methods */
enum sctm_unhook_method {
  /* disable the hook by instead calling the original */
  SCTM_UNHOOK_METHOD_DISABLE,
  /* replace the system call table entry with its original value */
  SCTM_UNHOOK_METHOD_REPLACE
};
#ifndef CONFIG_KALLSYMS
/* histogram of system call handlers */
struct sctm_syscall_handler_hist {
  size_t count;
  /* hash table-based histogram */
  struct sctm_syscall_handler_histent {
    unsigned int count;
    sctm_syscall_handler_t handler;
  } hist[SCTM_TABLE_SIZE];
};
#endif
/* hooked system call */
struct sctm_hook {
  /* system call number (index within system call table) */
  unsigned long call;
  sctm_syscall_handler_t hook; /* hook function */
  int hooked; /* flag for whether the hook should provide new functionality */
  enum sctm_unhook_method unhook_method; /* how to unhook the system call */ 
  sctm_syscall_handler_t original;
};

/* linked stack of hooked system calls */
struct sctm_hook_lstack {
  struct sctm_hook *hook;
  struct sctm_hook_lstack *next;
};

/* iterator for `struct sctm_hook_lstack` */
struct sctm_hook_lstacki {
  struct sctm_hook_lstack *cur;
  struct sctm_hook_lstack *stack;
  int syscall_in_progress;
};

/* quick lookup (and recursion support) for hooked system calls */
struct sctm_hook_lstacki sctm__hook_registry[SCTM_TABLE_SIZE];
sctm_syscall_handler_t *sctm__table;

/* module cleanup */
static void __exit sctm__exit(void);

/* hook a system call */
int sctm_hook(struct sctm_hook *hook);

/* call the hook and/or the original */
asmlinkage long sctm__hook_wrapper(unsigned long arg0, unsigned long arg1,
  unsigned long arg2, unsigned long arg3, unsigned long arg4,
  unsigned long arg5);

/* module initialization */
static int __init sctm__init(void);

/* locate the system call table */
static int sctm__locate_sys_call_table(void);

/* return 0 (compiler workaround) */
static inline int sctm__return_0(void);

/* set a system call handler */
static int sctm__set_syscall_handler(const unsigned long call,
  const sctm_syscall_handler_t handler);
#ifndef CONFIG_KALLSYMS
/* place the sum-modulus hash for `buf` into `dest` */
static inline int sctm__sum_mod_hash(size_t *dest, const unsigned char *buf, size_t count,
  const unsigned short mod);
#endif
/* unhook a hooked system call */
int sctm_unhook(struct sctm_hook *hook);

/* unhook and deregister all hooked system calls */
int sctm_unhook_all(void);

MODULE_LICENSE("GPL");

module_exit(sctm__exit);
module_init(sctm__init);

#endif

