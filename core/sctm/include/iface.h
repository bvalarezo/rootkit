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

