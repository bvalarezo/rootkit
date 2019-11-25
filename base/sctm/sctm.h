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

#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <poison.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

/* system call table modification module */

#undef SCTM__X86_64_TABLE_SIZE

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
/*
the maximum number of syscalls
(obtained by liberally hand-consolidating "include/linux/syscalls.h"
and the `grep`ping for "^asmlinkage")
*/
#ifndef SCTM_MAX_SYSCALLS
#define SCTM_MAX_SYSCALLS 425
#endif
/* system call table size */
#ifndef SCTM_TABLE_SIZE
#define SCTM_TABLE_SIZE SCTM__X86_64_TABLE_SIZE
#endif
/* system call table size on x86_64 */
#define SCTM__X86_64_TABLE_SIZE 547

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
};

/* quick lookup (and recursion support) for hooked system calls */
struct sctm_hook_lstacki *sctm__hooked_table[SCTM_TABLE_SIZE];
sctm_syscall_handler_t *sctm__table;

/* module cleanup */
static void __exit sctm__exit(void);

/* hook and register a system call */
int sctm_hook(struct sctm_hook *hook);

/* hook a system call */
static int sctm__hook(struct sctm_hook *hook);

/* call the hook and/or the original */
asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0,
  unsigned long arg1, unsigned long arg2, unsigned long arg3,
  unsigned long arg4);

/* module initialization */
static int __init sctm__init(void);

/* locate the system call table */
static int sctm__locate_sys_call_table(void);

/* return 0 (compiler workaround) */
static int sctm__return_0(void);

/* set a system call handler */
static int sctm__set_syscall_handler(unsigned long call,
  sctm_syscall_handler_t handler);

/* unhook and possibly deregister a hooked system call */
int sctm_unhook(struct sctm_hook *hook);

/* unhook a hooked system call */
static int sctm__unhook(struct sctm_hook *hook);

/* unhook and deregister all hooked system calls */
int sctm_unhook_all(void);

MODULE_LICENSE("GPL");

module_exit(sctm__exit);
module_init(sctm__init);

#endif

