#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main()
{
  FILE *fd1 = fopen("task_3.txt", "w");
  if (fd1 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  FILE *fd2 = fopen("task_3.txt", "w");
  if (fd1 == NULL)
  {
    printf("%s", strerror(errno));
    return errno;
  }

  for(char c = 'a'; c <= 'z'; c++)
  {
    if (c % 2)
    {
      fprintf(fd1, "%c", c);
    }
    else
    {
      fprintf(fd2, "%c", c);
    }
  }

  fclose(fd1);
  fclose(fd2);

  return 0;
}
