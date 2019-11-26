#ifndef HIDE_H
#define HIDE_H

#include "sctm.h"

/* hide/show directory entries */

/* hide a directory entry */
int hide(const char *path);

/* initialization */
int hide_init(void);

/* show a directory entry */
int show(const char *path);

#endif

