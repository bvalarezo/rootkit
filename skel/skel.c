#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* module skeleton */

#define DRIVER_AUTHOR "author"
#define DRIVER_DESC "description"

/* module cleanup */
static void __exit skel_cleanup(void) {
}

/* module initialization */
static int __init skel_init(void) {
  return 0;
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);

module_exit(skel_exit);
module_init(skel_init);

