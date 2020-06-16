#include <syslog.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h> //umask
#include <unistd.h> //setsid
#include <stdio.h> //perror
#include <signal.h> //sidaction
#include <string.h>
#include <errno.h >
#include <sys/file.h>
#include <stdarg.h>

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define MAXLINE 4096

int lockfile(int fd)
{
    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_start = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

int already_running(void)
{
    int fd;
    char buf[16];

    fd = open(LOCKFILE, O_RDWR|O_CREAT, LOCKMODE);

    if (fd < 0)
    {
        syslog(LOG_ERR, "невозможно открыть %s: %s",
        LOCKFILE, strerror(errno));

        exit(1);
    }

    if (lockfile(fd) < 0)
    {
        if (errno == EACCES || errno == EAGAIN)
        {
            close(fd);

            return(1);
        }

        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s",
        LOCKFILE, strerror(errno));

        exit(1);
    }

    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf) + 1);

    return(0);
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[MAXLINE];
    vsnprintf(buf, MAXLINE-1, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), MAXLINE-strlen(buf)-1, ": %s",
        strerror(error));
    strcat(buf, "\n");
    fflush(stdout); /* в случае, когда stdout и stderr - одно и то же устройство */
    fputs(buf, stderr);
    fflush(NULL); /* сбрасывает все выходные потоки */
}

void err_quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    /*
    * Сбросить маску режима создания файла.
    */
    umask(0);
    /*
    * Получить максимально возможный номер дескриптора файла.
    */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
        err_quit("%s: невозможно получить максимальный номер дескриптора ", cmd);
    /*
    * Стать лидером нового сеанса, чтобы утратить управляющий терминал.
    */
    if ((pid = fork()) < 0)
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0) /* родительский процесс */
        exit(0);
    setsid();
    /*
    * Обеспечить невозможность обретения управляющего терминала в будущем.
    */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
        err_quit("%s: невозможно игнорировать сигнал SIGHUP", cmd);
    if ((pid = fork()) < 0)
        err_quit("%s: ошибка вызова функции fork", cmd);
    else if (pid != 0) /* родительский процесс */
        exit(0);
    /*
    * Назначить корневой каталог текущим рабочим каталогом,
    * чтобы впоследствии можно было отмонтировать файловую систему.
    */
    if (chdir("/") < 0)
        err_quit("%s: невозможно сделать текущим рабочим каталогом /", cmd);
    /*
    * Закрыть все открытые файловые дескрипторы.
    */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
    /*
    * Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
    */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    /*
    * Инициализировать файл журнала.
    */
    openlog(cmd, LOG_CONS, LOG_DAEMON);

    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d",
        fd0, fd1, fd2);

        exit(1);
    }
}
