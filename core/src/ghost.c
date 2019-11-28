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
#include "ghost.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

static struct sctm *ghost__sctm = NULL;

/* remove a user from "/etc/passwd" and "/etc/shadow" */
int ghost(const uid_t uid) {
  return -EINVAL;///////////////////////
}

int ghost_exit(void) {
  if (ghost__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(ghost__sctm))
    return -EINVAL;
  /////////////////////////////////
  return 0;
}

int ghost_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(ghost__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  ghost__sctm = sctm;
  //////////////////////////////////////////
  return 0;
}

/* restore a user in "/etc/passwd" and "/etc/shadow" */
int unghost(const uid_t uid) {
  return -EINVAL;////////////////////////////////
}

