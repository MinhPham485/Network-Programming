#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(client, (struct sockaddr*)&addr, sizeof(addr));

    char buf[1024];

    while (1) {
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);

        int n = recv(client, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = 0;
            printf("Received: %s\n", buf);
        }
    }
}