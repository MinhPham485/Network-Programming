#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(client, (struct sockaddr *)&addr, sizeof(addr));

    printf("Connected to server\n");

    if (fork() == 0)
    {
        char buf[1024];

        while (1)
        {
            fgets(buf, sizeof(buf), stdin);

            send(client, buf, strlen(buf), 0);
        }
    }
    else
    {
        char buf[1024];

        while (1)
        {
            int ret = recv(client, buf, sizeof(buf)-1, 0);

            if (ret <= 0)
                break;

            buf[ret] = 0;

            printf("Server: %s", buf);
        }
    }

    close(client);

    return 0;
}