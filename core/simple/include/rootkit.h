#ifndef ROOTKIT_H
#define ROOTKIT_H

#include "elevate.h"
#include "ghost.h"
#include "hide.h"
#include "iface.h"
#include "sctm.h"

/* rootkit */

#define SCTM_INIT_POST_HOOK() rootkit__init()

/* initialization */
void rootkit_init(void);

#endif

