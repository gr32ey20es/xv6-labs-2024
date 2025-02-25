#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // your code here.  you should write the secret to fd 2 using write
  // (e.g., write(2, secret, 8)
  
  char *end;

  end = sbrk(PGSIZE*40);
  fprintf (2, "%s", end + 16 * PGSIZE + 32);
  
  exit(1);
}
