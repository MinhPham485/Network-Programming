#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc > 2) {
        printf("Usage: %s [PORT]\n", argv[0]);
        return 1;
    }

    int port = 8080;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server listening on port %d...\n", port);

    int client = accept(listener, NULL, NULL);

    char buf[1024];
    const char pattern[] = "0123456789";
    int state = 0;
    int count = 0;
    int n;

    while ((n = recv(client, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = 0;
        printf("Received: %s\n", buf);

        for (int i = 0; i < n; i++) {
            if (buf[i] == pattern[state]) {
                state++;
                if (state == 10) {
                    count++;
                    state = 0;
                }
            } else if (buf[i] == pattern[0]) {
                state = 1;
            } else {
                state = 0;
            }
        }

        printf("Count = %d\n", count);
    }

    close(client);
    close(listener);
}