#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define BUF_SIZE 1024

typedef struct {
    int fd;
    int logged_in;
    char client_id[50];
    char client_name[50];
} Client;

void remove_client(Client clients[], int *nClients, int i) {
    close(clients[i].fd);
    if (i < *nClients - 1) clients[i] = clients[*nClients - 1];
    (*nClients)--;
}

void trim(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = 0;
        n--;
    }
}

int parse_login(char *buf, char *id, char *name) {
    char *p = strchr(buf, ':');
    if (p == NULL) return 0;

    *p = 0;
    p++;

    while (*p == ' ') p++;

    trim(buf);
    trim(p);

    if (strlen(buf) == 0 || strlen(p) == 0) return 0;
    if (strchr(buf, ' ') != NULL) return 0;
    if (strchr(p, ' ') != NULL) return 0;

    strcpy(id, buf);
    strcpy(name, p);
    return 1;
}

void broadcast(Client clients[], int nClients, int sender_fd, const char *msg) {
    for (int i = 0; i < nClients; i++) {
        if (clients[i].fd != sender_fd) {
            send(clients[i].fd, msg, strlen(msg), 0);
        }
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket");
        return 1;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("bind");
        close(listener);
        return 1;
    }

    if (listen(listener, 5) != 0) {
        perror("listen");
        close(listener);
        return 1;
    }

    Client clients[MAX_CLIENTS];
    int nClients = 0;
    char buf[BUF_SIZE];

    printf("Chat server listening on port %d...\n", PORT);

    while (1) {
        fd_set fdread;
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);

        int maxfd = listener;
        for (int i = 0; i < nClients; i++) {
            FD_SET(clients[i].fd, &fdread);
            if (clients[i].fd > maxfd) maxfd = clients[i].fd;
        }

        int ret = select(maxfd + 1, &fdread, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(listener, &fdread)) {
            int client = accept(listener, NULL, NULL);
            if (client >= 0) {
                if (nClients < MAX_CLIENTS) {
                    clients[nClients].fd = client;
                    clients[nClients].logged_in = 0;
                    clients[nClients].client_id[0] = 0;
                    clients[nClients].client_name[0] = 0;
                    nClients++;

                    char *msg = "Nhap theo dung cu phap: client_id: client_name\n";
                    send(client, msg, strlen(msg), 0);
                    printf("New client connected: %d\n", client);
                } else {
                    char *msg = "Server full\n";
                    send(client, msg, strlen(msg), 0);
                    close(client);
                }
            }
        }

        for (int i = 0; i < nClients; i++) {
            if (!FD_ISSET(clients[i].fd, &fdread)) continue;

            ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
            if (ret <= 0) {
                printf("Client %d disconnected\n", clients[i].fd);
                remove_client(clients, &nClients, i);
                i--;
                continue;
            }

            buf[ret] = 0;
            trim(buf);

            if (!clients[i].logged_in) {
                char id[50], name[50];
                char temp[BUF_SIZE];
                strcpy(temp, buf);

                if (parse_login(temp, id, name)) {
                    strcpy(clients[i].client_id, id);
                    strcpy(clients[i].client_name, name);
                    clients[i].logged_in = 1;

                    char msg[200];
                    snprintf(msg, sizeof(msg), "Dang ky thanh cong: %s (%s)\n", id, name);
                    send(clients[i].fd, msg, strlen(msg), 0);
                } else {
                    char *msg = "Sai cu phap. Nhap lai: client_id: client_name\n";
                    send(clients[i].fd, msg, strlen(msg), 0);
                }
            } else {
                char out[1200];
                snprintf(out, sizeof(out), "%s: %s\n", clients[i].client_id, buf);
                broadcast(clients, nClients, clients[i].fd, out);
                printf("%s", out);
            }
        }
    }

    close(listener);
    return 0;
}