#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 256
#define MAX_CONNECT 1024
#define ANSWER "OK"
#define SIZE_ANSWER 2

typedef struct client client_t;
struct client {
    int fd;
    int pid;
};

int new_connection(int sock, client_t clients[FD_SETSIZE], fd_set *all_set,
        fd_set *reset, int *max_fd, int *max_idx);
int clients_handler(client_t clients[FD_SETSIZE], fd_set *all_set,
        fd_set *reset, int *max_idx);

int main(int argc, char ** argv)
{
    int sock, newsock, port;
    client_t clients[FD_SETSIZE];
    int max_fd, max_idx;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2)
    {
        fprintf(stderr,"usage: %s <port_number>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (socket < 0)
    {
        printf("socket() failed: %d\n", errno);
        return EXIT_FAILURE;
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    port = atoi(argv[1]);
    serv_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
    {
        printf("bind() failed: %d\n", errno);
        return EXIT_FAILURE;
    }

    printf("Server is running.\n");

    listen(sock, MAX_CONNECT);

    max_fd = sock;
    max_idx = -1;

    fd_set reset, all_set;
    FD_ZERO(&all_set);
    FD_SET(sock, &all_set);

    for (int i = 0; i < FD_SETSIZE; i++)
    {
        clients[i].fd = -1;
    }

    while (1)
    {
        reset = all_set;

        select(max_fd + 1, &reset, NULL, NULL, NULL);

        if (new_connection(sock, clients, &all_set, &reset, &max_fd,
                    &max_idx) != 0)
        {
            return EXIT_FAILURE;
        }
        if (clients_handler(clients, &all_set, &reset, &max_idx) != 0)
        {
            return EXIT_FAILURE;
        }
    }

    close(sock);
}

int new_connection(int sock, client_t clients[FD_SETSIZE], fd_set *all_set,
        fd_set *reset, int *max_fd, int *max_idx)
{
    if (FD_ISSET(sock, reset))
    {
        int conn_fd = accept(sock, NULL, NULL);
        if (conn_fd < 0)
        {
            printf("accept() failed: %d\n", errno);
            return EXIT_FAILURE;
        }

        int idx;
        for (idx = 0; idx < FD_SETSIZE; idx++)
        {
            if (clients[idx].fd < 0)
            {
                clients[idx].fd = conn_fd;
                break;
            }
        }
        if (idx == FD_SETSIZE)
        {
            perror("Maximum number of clients reached\n");
            return EXIT_FAILURE;
        }
        if (idx > *max_idx)
        {
            *max_idx = idx;
        }
        if (conn_fd > *max_fd)
        {
            *max_fd = conn_fd;
        }

        char pid[BUF_SIZE];
        read(conn_fd, pid, BUF_SIZE);
        clients[idx].pid = atoi(pid);

        FD_SET(conn_fd, all_set);
        printf("New client: %d\n", clients[idx].pid);
    }

    return 0;
}

int clients_handler(client_t clients[FD_SETSIZE], fd_set *all_set,
        fd_set *reset, int *max_idx)
{
    char buf[BUF_SIZE];
    int msg_size = 0;
    for (int i = 0; i <= *max_idx; i++)
    {
        if (clients[i].fd == -1)
        {
            continue;
        }

        if (FD_ISSET(clients[i].fd, reset))
        {
            msg_size = read(clients[i].fd, buf, BUF_SIZE);
            if (msg_size == 0)
            {
                close(clients[i].fd);
                FD_CLR(clients[i].fd, all_set);
                clients[i].fd = -1;
                printf("Client %d disconnected\n", clients[i].pid);
            }
            else
            {
                write(clients[i].fd, ANSWER, SIZE_ANSWER);
                buf[msg_size] = '\0';
                printf("Message from %d client: %s", clients[i].pid, buf);
            }
        }
    }

    return 0;
}
