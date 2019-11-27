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

/*
necessary definitions are included by defining
`SCTM_INCLUDE` at compile time
*/

#include "test.h"

/* simple linux system call hook test using `sctm` */

static struct sctm_hook my__hook = {
  .call = 102, /* `sys_getuid` */
  /* `.hook` will be set later on */
  /* `.hooked` is zeroed by the compiler */
  .unhook_method = SCTM_UNHOOK_METHOD_REPLACE
};
static const char *my__name = "test";

void my_exit(void) {
  printk(KERN_INFO "[%s]: In `my_exit` (%p).\n", my__name, &my_exit);
}

unsigned long my_hook_func(unsigned long arg0, unsigned long arg1,
    unsigned long arg2, unsigned long arg3, unsigned long arg4,
    unsigned long arg5) {
  printk(KERN_INFO "[%s]: In `my_hook_func` (%p).\n", my__name,
    &my_hook_func);
  return my__hook.hooked
    ? (*my__hook.original)(arg0, arg1, arg2, arg3, arg4, arg5)
    : -EINVAL;
}

int my_init(void) { 
  printk(KERN_INFO "[%s]: In `my_init` (%p).\n", my__name, &my_init);
  my__hook.hook = (sctm_syscall_handler_t) &my_hook_func;
  return sctm_hook(&my__hook);
}

