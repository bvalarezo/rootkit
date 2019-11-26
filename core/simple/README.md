# README
## Extending the Base Code
The base code is meant to be extended, in fact:
it barely does anything on its own.
To run your own code with the module,
there are 4 hooks (which are called in the following order):
1. `SCTM_INIT_PRE_HOOK`
2. `SCTM_INIT_POST_HOOK`
3. `SCTM_EXIT_PRE_HOOK`
and
4. `SCTM_EXIT_POST_HOOK`

The first 2 are called after the module is loaded,
and the other 2 are called before the module is unloaded.

### Hooking the System Call Table with `sctm`
Using `sctm`, this boils down to 3 basic steps:
1. represent your system call handler as `struct sctm_hook`
2. write an initialization function to call `sctm_hook` with your handler
3. define `SCTM_INIT_POST_HOOK` to a function or function-like macro containing the hooking code.

e.g.
```
/* simple code to hook a system call using `sctm` */

#define SCTM_INIT_POST_HOOK() my_init()

unsigned long my_hook_func(void) {
  return 0; /* r0073d */
}

struct sctm_hook my_hook = {
  .call = 102, /* `getuid` */
  .hook = (sctm_syscall_handler_t) &my_hook_func,
  .unhook_method = SCTM_UNHOOK_METHOD_REPLACE /* `SCTM_UNHOOK_METHOD_DISABLE` is preferable if we don't anticipate unloading the module */
};

void my_init() {
  printk(sctm_hook(&my_hook) ? KERN_WARN "Failed to add `my_hook`." : KERN_MSG "Added `my_hook`.");
}

```

