#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#define SOCK_NAME "socket.soc"
#define BUF_SIZE 256

int main(int argc, char ** argv)
{
    int sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    char buf[BUF_SIZE];
    char msg[BUF_SIZE];
    struct sockaddr srvr_name;

    if (sock < 0)
    {
    perror("socket failed");
    return EXIT_FAILURE;
    }

    srvr_name.sa_family = AF_UNIX;
    strcpy(srvr_name.sa_data, SOCK_NAME);

    printf("message: ");
    scanf("%255s",  msg);

    sprintf(buf, "client %d: %s", getpid(), msg);

    sendto(sock, buf, strlen(buf), 0, &srvr_name,
           strlen(srvr_name.sa_data) + sizeof(srvr_name.sa_family));

    return 0;
}
