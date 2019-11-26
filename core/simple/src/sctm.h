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

#include <asm/paravirt.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <linux/version.h>

/* system call table modification module */

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

/* hooked system call */
struct sctm_hook {
  /* system call number (index within system call table) */
  unsigned long call;
  sctm_syscall_handler_t hook; /* hook function */
  int hooked; /* flag for whether the hook should provide new functionality */
  enum sctm_unhook_method unhook_method; /* how to unhook the system call */ 
  sctm_syscall_handler_t original;
};

/* hook a system call */
int sctm_hook(struct sctm_hook *hook);

/* call the hook and/or the original */
asmlinkage long sctm__hook_wrapper(unsigned long call, unsigned long arg0,
  unsigned long arg1, unsigned long arg2, unsigned long arg3,
  unsigned long arg4);

/* unhook a hooked system call */
int sctm_unhook(struct sctm_hook *hook);

/* unhook and deregister all hooked system calls */
int sctm_unhook_all(void);

MODULE_LICENSE("GPL");

#endif

