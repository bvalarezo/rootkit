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
#ifndef ROOTKIT_H
#define ROOTKIT_H

#include "elevate.h"
#include "ghost.h"
#include "hide.h"
#include "iface.h"
#include "sctm.h"

/* rootkit */

#define SCTM_INIT_POST_HOOK() rootkit_init()

/* initialization */
int rootkit_init(void);

#endif

