#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    char buf[256];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int ret = recvfrom(receiver, buf, sizeof(buf)-1, 0,
            (struct sockaddr *)&client_addr, &client_len);

        if (ret <= 0) break;

        buf[ret] = 0;

        printf("Received from %s:%d: %s\n",
            inet_ntoa(client_addr.sin_addr),
            ntohs(client_addr.sin_port),
            buf);

        // gui lai cho client
        sendto(receiver, buf, ret, 0,
            (struct sockaddr *)&client_addr, client_len);
    }
}