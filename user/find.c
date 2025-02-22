#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

char *pattern;
uint npattern;

// name must be end with slash
int
pattern_match (char *name)
{
  int cnt, i;
  
  cnt = -1;
  while (name[++cnt] != '/');
  
  if (cnt < npattern)
    return 0;

  for (i = 0; i < npattern; ++i)
    {
      if (name[i] != pattern[i])
        return 0;
    }

  return 1;
}

// npos: the position of the file's name included 
// in fpath (file's path) that ends with slash
void
find (char *fpath, int npos)
{
  struct dirent di;
  struct stat st;
  
  char buf[128];
  char *name;

  uint disize;
  int fd;
  int cnt, i;

  if ((fd = open (fpath, O_RDONLY)) < 0)
    {
      fprintf (2, "open: %s does't exist\n", fpath);
      exit (1);
    }
  
  if (fstat (fd, &st) < 0)
    {
      fprintf (2, "fstat: %s does't exist\n", fpath);
      close (fd); 
      exit (1);
    }
  
  switch (st.type)
    {
      case T_DEVICE:
      case T_FILE:
        name = fpath + npos;
        if (pattern_match (name))
          {
            cnt = npos;
            while (fpath [cnt++] != '/');
            --cnt;

            write (1, fpath, cnt);
            write (1, "\n", 1);
          }
        break;
      
      case T_DIR:
        disize = sizeof (di);
        read (fd, &di, disize); //. file
        read (fd, &di, disize); //.. file
          
        while (read (fd, &di, disize) == disize)
          {
            i = 0;
            cnt = npos;

            strcpy (buf, fpath);
            while (buf[cnt++] != '/');

            while (di.name[i] != 0)
              {
                buf[cnt+i] = di.name[i];
                ++i;
              }
            buf[cnt+i] = '/';
            buf[cnt+i+1] = 0;
            
            if (strcmp (buf, ".//") != 0)
              find (buf, cnt);
          }
    }

  close (fd);
}

int
main (int argc, char *argv[])
{
  char buf[128];
  int npos, i;

  if (argc != 3)
    {
      fprintf (2, "usage: find dir/file pattern\n");
      exit (1);
    }
  
  strcpy (buf, argv[1]);
  pattern = argv[2];
  npattern = strlen (pattern);

  npos = i = 0;
  while (buf[i] != 0)
    {
      if (buf[i] == '/' && buf[i+1] != 0)
        npos = i + 1;
        
      ++i;
    }
  
  i--;
  if (buf[i] != '/')
    {
      buf[i+1] = '/';
      buf[i+2] = 0;
    }
  
  find (buf, npos);
  
  exit (0);
}
