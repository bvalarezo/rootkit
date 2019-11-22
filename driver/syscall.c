#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* execute a system call from the command line */

enum endian {
  ENDIAN_BIG,
  ENDIAN_LITTLE
};

/* return the native endianness */
enum endian endianness(void) {
  short test;

  test = 0xFF;
  return *((char *) &test) ? ENDIAN_LITTLE : ENDIAN_BIG;
}

/* interpret a hexadecimal character as a nibble */
int hton(unsigned char *dest, char c) {
  c = tolower(c);

  if (c < '0'
      || (c > '9'
        && c < 'a')
      || c > 'f') {
    return -EINVAL;
  }
  *dest = c < 'a' ? c - '0' : c - 'a' + 10;
  return 0;
}

/* interpret a string as an unsigned long in hexadecimal (native endianness), and put the result into `dest` */
int htoul(unsigned long *dest, const char *a) {
  unsigned char c;
  int i;
  int retval;
  int shift;

  if (a == NULL)
    return -EFAULT;

  if (strlen(a) < 2
      || strlen(a) > 2 + sizeof(*dest)
      || *a != '0'
      || (a[1] != 'X'
        && a[1] != 'x'))
    return -EINVAL;

  if (dest == NULL)
    return -EFAULT;
  a += 2; /* ignore prefix */

  /* read into dest */

  *dest = 0;

  for (i = strlen(a) - 1, shift = 0; i >= 0; i--, shift += 4) {
    retval = hton(&c, a[i]);

    if (retval)
      return retval;
    *dest |= c << shift;
  }
  return 0;
}

int main(int argc, char **argv) {
  unsigned long args[6];
  int i;

  if (!argc) {
    printf("No argument vector (this shouldn't happen)?\n");
    return 1;
  }

  if (argc < 2) {
    printf("%s - make a system call\n" \
      "Usage: %s [ULONG | 0xHEXULONG | @CARRAY] ...\n", *argv, *argv);
    return 1;
  }

  /* fill slots */

  for (i = 1; i < argc; i++) {
    if (*argv[i] == '0'
        && tolower(*(argv[i] + 1)) == 'x') {
      /* seems like it's a hex argument */
      
      if (htoul(&args[i - 1], argv[i])) {
        printf("Saw \"0x\" prefix, but got malformed hex: \"%s\"\n", argv[i]);
        return 1;
      }
    } else if (*argv[i] == '@') {
      /* got string classification prefix (mnemonic: "@" for array) */

      args[i - 1] = (unsigned long) &argv[i][1];
    } else {
      /* probably an integer argument */

      args[i - 1] = atoi(argv[i]);
    }
  }

  /* fill remaining slots */

  for (; i < 7; i++)
    args[i - 1] = 0L;

  /* confirm */

  printf("Execute `syscall(");
  
  for (i = 0; i < 5; i++)
    printf("0x%x, ", args[i]);
  printf("0x%x)`? [Y/n] ", args[i]);
  fflush(stdout);

  if (tolower(getchar()) != 'y')
    abort();

  /* execute the syscall */

  printf("\t->%u\n.", syscall(args[0], args[1], args[2], args[3], args[4], args[5]));
}
