#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define BUF_SIZE 0x1000

static char* param_stat[] = {
    "pid", "comm", "state", "ppid", "pgrp", "session",
    "tty_nr", "tpgid", "flags", "minflt", "cminflt",
    "majflt", "cmajflt", "utime","stime", "cutime",
    "cstime", "priority", "nice", "num_threads",
    "itrealvalue", "starttime", "vsize", "rss", "rsslim",
    "startcode", "endcode", "startstack","kstkesp",
    "kstkeip", "signal", "blocked", "sigignore","sigcatch",
    "wchan", "nswap", "сnswap", "exit_signal", "processor",
    "rt_priority", "policy", "delayacct_blkio_ticks",
    "guest_time", "cguest_time", "start_data", "end_data",
    "start_brk", "arg_start", "arg_end", "env_start", "env_end",
    "exit_code"
};

void print_environ()
{
  char buf[BUF_SIZE] = {0};
  int len, i;
  FILE *fd = fopen("/proc/self/environ", "r");
  if (fd == NULL)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((len = fread(buf, 1, BUF_SIZE, fd)) > 0)
  {
    for (i = 0; i < len; i++)
      if (buf[i] == 0)
      {
        buf[i] = 10;
      }
    buf[len] = 0;
    printf("%s", buf);
  }

  fclose(fd);
}

void print_file(char *path)
{
    char buf[BUF_SIZE] = {0};
    char *pch;
    int i = 0;
    FILE *f = fopen(path, "r");
    if (f == NULL)
    {
        printf("%s", strerror(errno));
        exit(errno);
    }

    fread(buf, 1, BUF_SIZE, f);
    pch = strtok(buf, " ");

    while (pch)
    {
        if (strcmp("/proc/self/stat", path) == 0)
        {
            printf("%s=", param_stat[i++]);
        }
        printf("%s\n", pch);
        pch = strtok(NULL, " ");
    }

    fclose(f);
}

void print_fd()
{
  struct dirent *dirp;
  DIR *dp;
  char path[BUF_SIZE];
  char str[BUF_SIZE];

  if (!(dp = opendir("/proc/self/fd/")))
	{
    printf("%s", strerror(errno));
    exit(errno);
  }

  while ((dirp = readdir(dp)))
  {
    if ((strcmp(dirp->d_name, ".") != 0) && (strcmp(dirp->d_name, "..") != 0))
    {
      sprintf(path, "%s%s", "/proc/self/fd/", dirp->d_name);
      readlink(path, str, BUF_SIZE);
      printf("%s -> %s\n", dirp->d_name, str);
    }
  }

  if (closedir(dp) < 0)
  {
    printf("%s", strerror(errno));
    exit(errno);
  }
}

int main()
{
    printf("Environ:\n");
    print_environ();
    printf("\n");

    printf("Cmdline:\n");
    print_file("/proc/self/cmdline");
    printf("\n");

    printf("Stat:\n");
    print_file("/proc/self/stat");
    printf("\n");

    printf("fd:\n");
    print_fd();
    printf("\n");

    return 0;
}
