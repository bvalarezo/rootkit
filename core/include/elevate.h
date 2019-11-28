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
#ifndef ELEVATE_H
#define ELEVATE_H

#include <linux/err.h>
#include <linux/errno.h>

#include "sctm.h"

/* elevate/drop EUIDs */

/* drop a process's EUID */
int drop(const pid_t pid);

/* elevate a process's EUID */
int elevate(const pid_t pid);

int elevate_exit(void);

int elevate_init(struct sctm *sctm);

#endif

