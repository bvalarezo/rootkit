#include "ghost.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

/* remove a user from "/etc/passwd" and "/etc/shadow" */
int ghost(const uid_t uid) {
  return -EINVAL;///////////////////////
}

/* initialization */
int ghost_init(void) {
  return 0;//////////////////////////////
}

/* restore a user in "/etc/passwd" and "/etc/shadow" */
int unghost(const uid_t uid) {
  return -EINVAL;////////////////////////////////
}

