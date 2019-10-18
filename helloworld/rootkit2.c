#include <linux/module.h>
#include <asm/unistd.h>
#include <linux/highmem.h>
 
unsigned long *sys_call_table = (unsigned long*) 0xc1538160;
 
asmlinkage int (*real_open)(const char* __user, int, int);
 
asmlinkage int custom_open(const char* __user file_name, int flags, int mode)
{
      printk("hooked: open(\"%s\", %X, %X)\n", file_name,
                                                    flags,
                                                    mode);
  return real_open(file_name, flags, mode);
}  
 
int make_rw(unsigned long address)
{  
   unsigned int level;
   pte_t *pte = lookup_address(address,&level);
   if(pte->pte &~ _PAGE_RW)
      pte->pte |= _PAGE_RW;
   return 0;
}
 
int make_ro(unsigned long address)
{
   unsigned int level;
   pte_t *pte = lookup_address(address, &level);
   pte->pte = pte->pte &~ _PAGE_RW;
   return 0;
}
 
static int hello_init(void){
  make_rw((unsigned long)sys_call_table);
  real_open = (void*)*(sys_call_table + __NR_open);
  *(sys_call_table + __NR_open) = (unsigned long)custom_open;
  make_ro((unsigned long)sys_call_table);
  return 0;
}
 
static void hello_exit(void){
  make_rw((unsigned long)sys_call_table);
  *(sys_call_table + __NR_open) = (unsigned long)real_open;
  make_ro((unsigned long)sys_call_table);
}
 
module_init(hello_init);
module_exit(hello_exit);
 
MODULE_LICENSE("GPL");