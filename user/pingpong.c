#include "kernel/types.h"
#include "user/user.h"

int
main (int argc, char *argv[])
{
  int p1[2], p2[2];
  char buf[1];

  if (pipe (p1) < 0 || pipe (p2) < 0)
    {
      fprintf (2, "pipe: fatal error");
      exit (1);
    }

  switch (fork ())
    {
      case -1:
        fprintf (2, "fork: fatal error");
        exit (1);

      case 0:
        close (p1[1]);
        close (p2[0]);
        read (p1[0], buf, sizeof (buf));

        if (buf[0] == 'i')
          {
            fprintf (1, "%d: received ping\n", getpid ());
            write (p2[1], "o", 1);
            close (p1[0]);
            close (p2[1]);
          }
        else
          {
            fprintf (2, "Something's wrong!");
          }
        exit (0);

      default:
        close (p1[0]);
        close (p2[1]);
        write (p1[1], "i", 1);
        
        read (p2[0], buf, sizeof (buf));
        if (buf[0] == 'o')
          {
            fprintf (1, "%d: received pong\n", getpid ());
            close (p1[1]);
            close (p2[0]);
          }
        else 
          {
            fprintf (2, "Something's wrong!");
          }
        exit (0);
    }
}

