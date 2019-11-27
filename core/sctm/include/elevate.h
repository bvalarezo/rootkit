#ifndef ELEVATE_H
#define ELEVATE_H

#include <linux/errno.h>

#include "sctm.h"

/* elevate/drop EUIDs */

/* drop a process's EUID */
int drop(const pid_t pid);

/* elevate a process's EUID */
int elevate(const pid_t pid);

/* initialization */
int elevate_init(void);

#endif

