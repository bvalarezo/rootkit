#ifndef TEST_H
#define TEST_H

#include <linux/errno.h>
#include <linux/printk.h>

/* configure `sctm` */

#define SCTM_EXIT_POST_HOOK() my_exit()
#define SCTM_INIT_POST_HOOK() my_init()
#include "sctm.h"

/* simple linux system call hook test using `sctm` */

/* exit */
void my_exit(void);

/* initialization */
int my_init(void);

#endif

