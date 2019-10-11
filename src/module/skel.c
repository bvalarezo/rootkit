#include "module/module.h"

/* module skeleton */

/* module cleanup */
static void __exit do_exit(void) {
}

/* module initialization */
static int __init do_init(void) {
  return 0;
}

module_exit(skel_exit);
module_init(skel_init);

