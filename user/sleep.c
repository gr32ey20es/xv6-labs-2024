#include "kernel/types.h"
#include "user/user.h"

int
main (int argc, char *argv[])
{
  int ticks;

  if (argc != 2 || (ticks = atoi (argv[1])) <= 0)
    {
      fprintf (2, "Usage: sleep positive_integer\n");
      exit (1);
    }
  
  sleep (ticks);
  exit (0);
}

