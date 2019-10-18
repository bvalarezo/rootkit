#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

/* module skeleton */

/* module cleanup */
static void __exit skel_cleanup(void) {
}

/* module initialization */
static int __init skel_init(void) {
  return 0;
}

module_exit(skel_exit);
module_init(skel_init);

