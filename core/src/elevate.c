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
#include "elevate.h"

/* elevate/drop EUIDs */

static struct sctm *elevate__sctm = NULL;

/* drop a process's EUID */
int drop(const pid_t pid) {
  return -EINVAL;//////////////////////////////
}

/* elevate a process's EUID */
int elevate(const pid_t pid) {
  return -EINVAL;//////////////////////////////
}

int elevate_exit(void) {
  if (elevate__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(elevate__sctm))
    return -EINVAL;
  /////////////////////////////////
  return 0;
}

int elevate_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(elevate__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  elevate__sctm = sctm;
  ////////////////////////////////
  return 0;
}

