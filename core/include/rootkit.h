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

#define __LINUX__
//#define MODULE

#include <linux/err.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#ifndef ROOTKIT_DEBUG
#include <linux/printk.h>
#endif
#include <linux/version.h>

#include "elevate.h"
#include "ghost.h"
#include "hide.h"
#include "iface.h"
#include "sctm.h"

/* rootkit */

#ifdef DEBUG

#undef ROOTKIT_DEBUG
#undef ROOTKIT__DEBUG_BASE
#undef ROOTKIT_ERROR
#undef ROOTKIT_NAME

#define ROOTKIT_DEBUG(...) ROOTKIT__DEBUG_BASE(KERN_INFO, __VA_ARGS__)
#define ROOTKIT__DEBUG_BASE(p, f, ...) do { \
    printk(p "[" ROOTKIT_NAME ":%s:%d]: " f, __FILE__, __LINE__, ##__VA_ARGS__); \
  } while (0)
#define ROOTKIT_ERROR(...) ROOTKIT__DEBUG_BASE(KERN_ERR, __VA_ARGS__)
#define ROOTKIT_NAME "rootkit"

#endif

#endif

