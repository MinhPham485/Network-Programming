#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

void signal_handler(int sig)
{
    wait(NULL);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));

    listen(listener, 5);

    printf("Server listening on port 9000...\n");

    signal(SIGCHLD, signal_handler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);

        printf("Client connected: %d\n", client);

        if (fork() == 0)
        {
            close(listener);

            char buf[1024];

            while (1)
            {
                int ret = recv(client, buf, sizeof(buf)-1, 0);

                if (ret <= 0)
                    break;

                buf[ret] = 0;

                printf("Client %d: %s", client, buf);

                send(client, buf, strlen(buf), 0);
            }

            close(client);

            printf("Client disconnected\n");

            exit(0);
        }

        close(client);
    }

    close(listener);

    return 0;
}