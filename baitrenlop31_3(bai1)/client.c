#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUF_SIZE 1024

int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblock(sock);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        select(sock + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sock, &readfds)) {
            char buf[BUF_SIZE];
            int n = recv(sock, buf, sizeof(buf) - 1, 0);
            if (n <= 0) break;

            buf[n] = 0;
            printf("%s", buf);

            if (strstr(buf, "Nhap Ho ten:") || strstr(buf, "Nhap MSSV:")) {
                char input[256];
                fgets(input, sizeof(input), stdin);
                send(sock, input, strlen(input), 0);
            }
        }
    }

    close(sock);
    return 0;
}