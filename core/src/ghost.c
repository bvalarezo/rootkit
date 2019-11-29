#include "ghost.h"

/* hide/show a user in "/etc/passwd" and "/etc/shadow" */

static struct sctm *ghost__sctm = NULL;

/* remove a user from "/etc/passwd" and "/etc/shadow" */
int ghost(const uid_t uid) {
  return -EINVAL;///////////////////////
}

int ghost_exit(void) {
  if (ghost__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(ghost__sctm))
    return -EINVAL;
  /////////////////////////////////
  return 0;
}

int ghost_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(ghost__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  ghost__sctm = sctm;
  //////////////////////////////////////////
  return 0;
}

/* restore a user in "/etc/passwd" and "/etc/shadow" */
int unghost(const uid_t uid) {
  return -EINVAL;////////////////////////////////
}

