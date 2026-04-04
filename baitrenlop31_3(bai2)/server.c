#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s port_s ip_d port_d\n", argv[0]);
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 1;

    set_nonblock(sock);
    set_nonblock(STDIN_FILENO);

    struct sockaddr_in local_addr, dest_addr, src_addr;
    socklen_t src_len = sizeof(src_addr);

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port_s);

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        close(sock);
        return 1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port_d);
    inet_pton(AF_INET, ip_d, &dest_addr.sin_addr);

    printf("UDP chat running at port %d -> %s:%d\n", port_s, ip_d, port_d);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int maxfd = sock > STDIN_FILENO ? sock : STDIN_FILENO;
        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buf[BUF_SIZE];
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                sendto(sock, buf, strlen(buf), 0,
                       (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            }
        }

        if (FD_ISSET(sock, &readfds)) {
            char buf[BUF_SIZE];
            int n = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                             (struct sockaddr *)&src_addr, &src_len);
            if (n > 0) {
                buf[n] = 0;
                printf("Peer: %s", buf);
                fflush(stdout);
            }
        }
    }

    close(sock);
    return 0;
}