#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define WORKERS 4

void worker_process(int listener)
{
    while (1)
    {
        int client = accept(listener, NULL, NULL);

        if (client < 0)
            continue;

        char buf[2048];

        recv(client, buf, sizeof(buf), 0);

        printf("HTTP Request:\n%s\n", buf);

        char response[] =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<h1>Hello from Prefork HTTP Server</h1>";

        send(client, response, strlen(response), 0);

        close(client);
    }
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));

    listen(listener, 10);

    printf("HTTP Server listening on port 8080...\n");

    for (int i = 0; i < WORKERS; i++)
    {
        if (fork() == 0)
        {
            worker_process(listener);
            exit(0);
        }
    }

    while (1)
        pause();

    close(listener);

    return 0;
}