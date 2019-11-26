#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>

/* define hooks ASAP */

#define SCTM_EXIT_POST_HOOK() my_exit()
#define SCTM_INIT_POST_HOOK() my_init()
#include "sctm.h"

/* simple linux system call hook test using `sctm` */

/* exit */
extern void my_exit(void);

/* initialization */
extern int my_init(void);

