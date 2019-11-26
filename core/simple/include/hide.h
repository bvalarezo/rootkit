#ifndef HIDE_H
#define HIDE_H

#include "sctm.h"

/* hide/show directory entries */

/* hide a directory entry */
int hide(const char *name);

/* initialization */
int hide_init(void);

/* show a directory entry */
int hide_show(const char *name);

#endif

