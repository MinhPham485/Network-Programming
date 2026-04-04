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
#define MAX_CLIENTS 100
#define BUF_SIZE 1024
#define DOMAIN "sis.hust.edu.vn"

typedef enum {
    WAIT_NAME,
    WAIT_MSSV
} State;

typedef struct {
    int fd;
    State state;
    char name[256];
    char buf[BUF_SIZE];
} Client;

int set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void trim(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = 0;
        n--;
    }
}

void lower_str(char *s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] += 32;
    }
}

void build_email(const char *fullname, const char *mssv, char *email) {
    char temp[256], parts[20][50];
    int count = 0;

    strncpy(temp, fullname, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = 0;

    char *token = strtok(temp, " ");
    while (token && count < 20) {
        strcpy(parts[count++], token);
        token = strtok(NULL, " ");
    }

    if (count == 0) {
        strcpy(email, "invalid@sis.hust.edu.vn");
        return;
    }

    char name[50] = "", prefix[3] = "";
    strcpy(name, parts[count - 1]);
    lower_str(name);

    prefix[0] = parts[0][0];
    prefix[1] = parts[0][1];
    prefix[2] = 0;
    lower_str(prefix);

    const char *tail = mssv;
    if (strlen(mssv) > 2) tail = mssv + 2;

    sprintf(email, "%s.%s%s@%s", name, prefix, tail, DOMAIN);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblock(listener);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);

    Client clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) clients[i].fd = -1;

    printf("Server listening on %d\n", PORT);

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int maxfd = listener;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd != -1) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(listener, &readfds)) {
            int client = accept(listener, NULL, NULL);
            if (client != -1) {
                set_nonblock(client);
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i].fd == -1) {
                        clients[i].fd = client;
                        clients[i].state = WAIT_NAME;
                        send(client, "Nhap Ho ten: ", 13, 0);
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd == -1) continue;
            if (!FD_ISSET(clients[i].fd, &readfds)) continue;

            char temp[256];
            int n = recv(clients[i].fd, temp, sizeof(temp) - 1, 0);

            if (n <= 0) {
                close(clients[i].fd);
                clients[i].fd = -1;
                continue;
            }

            temp[n] = 0;
            strcpy(clients[i].buf, temp);
            trim(clients[i].buf);

            if (clients[i].state == WAIT_NAME) {
                strcpy(clients[i].name, clients[i].buf);
                clients[i].state = WAIT_MSSV;
                send(clients[i].fd, "Nhap MSSV: ", 11, 0);
            } else {
                char email[256], out[512];
                build_email(clients[i].name, clients[i].buf, email);
                snprintf(out, sizeof(out), "Email: %s\n", email);
                send(clients[i].fd, out, strlen(out), 0);
                close(clients[i].fd);
                clients[i].fd = -1;
            }
        }
    }

    close(listener);
    return 0;
}