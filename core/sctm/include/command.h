#ifndef COMMAND_H
#define COMMAND_H

#include <linux/errno.h>

#include "sctm.h"

/* execute system commands */

/* execute a system command */
int command(const char *cmd);

/* initialization */
int command_init(void);

#endif

