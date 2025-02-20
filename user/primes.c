#include "kernel/types.h"
#include "user/user.h"

void 
sieve (void)
{
  int num, haschild, tmp;
  int p[2];

  num = haschild = 0;

  for (;;)
    {
      if (read (0, &tmp, 4) == 0)
        {
          if (haschild == 1) close (1);
          wait ((int *) 0);
          exit (0);
        }
      
      if (num == 0)
        {
          num = tmp;
          fprintf (2, "prime %d\n", num);
          continue;
        }

      if (tmp % num == 0)
        continue;

      if (haschild == 1)
        {
          write (1, &tmp, 4);
          continue;  
        }
      
      haschild = 1;

      if (pipe (p) < 0)
        {
          fprintf (2, "sieve's pipe: fatal error");
          exit (1);
        }
      
      switch (fork ())
        {
          case -1:
            fprintf (2, "sieve's fork: fatal error");
            exit (1);
          
          case 0:
            if (close (0) < 0)
              {
                fprintf (2, "sieve's closes: fatal error");
                exit (1);
              }

            if (dup (p[0]) != 0)
              {
                fprintf (2, "sieve's dup0: fatal error");
                exit (1);
              }

            if (close (p[0]) < 0 || close (p[1]) < 0)
              {
                fprintf (2, "sieve's closes: fatal error");
                exit (1);
              }

            num = haschild = 0;
            break;

          default:
            if (close (p[0]) < 0)
              {
                fprintf (2, "it must be a point");
                exit (1);
              }

            if (dup (p[1]) != 1)
              {
                fprintf (2, "sieve's dup1: fatal error");
                exit (1);
              }
              
            if (close (p[1]) < 0)
              {
                fprintf (2, "sieve's closes: fatal error");
                exit (1);
              }

            write (1, &tmp, 4); 
        }
    } 
}

int
main (int argc, char *argv[])
{
  int p[2];
  int num;

  // create a temporary pipe
  if (pipe (p) < 0)
    {
      fprintf (2, "pipe: fatal error");
      exit (1);
    }

  // redirect stdin and stdout to left pipe's read 
  // and right pipe's write respectively
  if (close (0) < 0 || close (1) < 0) 
    {
      fprintf (2, "closes: fatal error");
      exit (1);
    }

  if (dup (p[0]) != 0 || close (p[0]) < 0)
    {
      fprintf (2, "dup0: fatal error");
      exit (1);
    }

  if (dup (p[1]) != 1 || close (p[1]) < 0)
    {
      fprintf (2, "dup1: fatal error");
      exit (1);
    }
  
  // parent: first process
  // child: pipe process
  switch (fork ())
    {
      case -1:
        fprintf (2, "first process's fork: fatal error");
        exit (1);

      case 0:
        if (close (1) < 0) exit (1);
        sieve ();
        exit (0);

      default:
        if (close (0) < 0) exit (1);

        for (num = 2; num <= 280; ++num)
            write (1, &num, 4);
        
        if (close (1) < 0) exit (1);

        if (wait ((int *) 0) < 0) exit (1);
        exit (0);
    }
}
