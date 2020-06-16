#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#define BUF_SIZE 256

int main(int argc, char ** argv)
{
    int sock, port;
    int pid;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buf[BUF_SIZE];
    char buf_answer[BUF_SIZE];

    if (argc < 3)
    {
        fprintf(stderr,"usage: %s <hostname> <port_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid = getpid();

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        printf("socket() failed: %d", errno);
        return EXIT_FAILURE;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        printf("Host not found\n");
        return EXIT_FAILURE;
    }

    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr,
            server->h_length);
    port = atoi(argv[2]);
    serv_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr*) &serv_addr,
                sizeof(serv_addr)) < 0)
    {
        printf("connect() failed: %d", errno);
        return EXIT_FAILURE;
    }

    sprintf(buf, "%d", pid);
    write(sock, buf, BUF_SIZE);

    printf("Client %d:\n", pid);

    while (strcmp(buf, "break\n"))
    {
        memset(buf, 0, BUF_SIZE);
        printf("Message: ");
        fgets(buf, BUF_SIZE-1, stdin);
        write(sock, buf, strlen(buf));

        memset(buf_answer, 0, BUF_SIZE);
        read(sock, buf_answer, BUF_SIZE-1);
        printf("Answer: %s\n",buf_answer);
    }

    close(sock);
    return 0;
}
