#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int port = atoi(argv[1]);

    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server...\n");

    int client = accept(listener, NULL, NULL);

    char buf[1024];
    int n;

    while ((n = recv(client, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = 0;
        printf("%s", buf);
    }

    close(client);
}