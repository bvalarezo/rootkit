#ifndef ELEVATE_H
#define ELEVATE_H

#include "sctm.h"

/* elevate/drop EUIDs */

/* elevate a process's EUID */
int elevate(pid_t pid);

/* drop a process's EUID */
int elevate_drop(pid_t pid);

/* initialization */
int elevate_init(void);

#endif

