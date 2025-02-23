#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int 
main (int argc, char *argv[])
{
  char *arg[MAXARG+1];
  char buf[128];
  char ch;
  int cnt;

  cnt = 0;
  while (read (0, &ch, 1) == 1)
    {
      if (ch == '\n')
        {
          buf[cnt] = 0;
          switch (fork ())
            {
              case -1:  
                fprintf (2, "fork: fatal error");
                exit (1);

              case 0:
                for (cnt = 1; cnt < argc; ++cnt)
                  arg[cnt-1] = argv[cnt];

                arg[cnt-1] = buf;
                arg[cnt] = (char *) 0;

                exec (arg[0], arg);
                fprintf (2, "exec: fatal error");
                exit (1);
                
              default:
                wait((int *) 0);
                cnt = 0;
            }
        }
      else
        buf[cnt++] = ch;
    }

  exit (0);
}
