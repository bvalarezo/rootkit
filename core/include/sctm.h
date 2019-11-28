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

#ifndef CONFIG_KALLSYMS
#error "Need `kallsyms`."
#endif

#include <linux/err.h>
#include <linux/errno.h>
#include <asm/paravirt.h>
#include <linux/syscalls.h>

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

/* system call table modifier */
struct sctm {
  struct sctm_hook *hook_registry[SCTM_TABLE_SIZE];
  sctm_syscall_handler_t *table;
};

/* hooked system call */
struct sctm_hook {
  /* system call number (index within system call table) */
  unsigned long call;
  sctm_syscall_handler_t hook; /* hook function */
  int hooked; /* flag for whether the hook should provide new functionality */
  sctm_syscall_handler_t original;
};

int sctm_cleanup(struct sctm *sctm);

/* hook a system call */
int sctm_hook(struct sctm *sctm, struct sctm_hook *hook);

int sctm_init(struct sctm *sctm);

/* unhook a hooked system call */
int sctm_unhook(struct sctm *sctm, struct sctm_hook *hook);

/* unhook and deregister all hooked system calls */
int sctm_unhook_all(struct sctm *sctm);

#endif

