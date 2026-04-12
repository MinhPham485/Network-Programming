#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

#define MAX_CLIENTS 1020

typedef struct {
    int fd;
    int state;
    char user[50];
    char pass[50];
} Client;

int removeClient(Client *clients, int *nClients, int i) {
    close(clients[i].fd);
    if (i < *nClients - 1)
        clients[i] = clients[*nClients - 1];
    *nClients -= 1;
    return 0;
}

void trim(char *s) {
    int n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = 0;
        n--;
    }
}

int checkUserPass(const char *user, const char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (f == NULL) return 0;

    char u[50], p[50];
    while (fscanf(f, "%49s %49s", u, p) == 2) {
        if (strcmp(user, u) == 0 && strcmp(pass, p) == 0) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void sendFile(int client, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        char *msg = "Cannot open out.txt\n";
        send(client, msg, strlen(msg), 0);
        return;
    }

    char buf[256];
    int n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        send(client, buf, n, 0);
    }
    fclose(f);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))) {
        perror("setsockopt() failed");
        close(listener);
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        close(listener);
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        close(listener);
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    Client clients[MAX_CLIENTS];
    int nClients = 0;
    fd_set fdread;
    char buf[256];

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        for (int i = 0; i < nClients; i++) {
            FD_SET(clients[i].fd, &fdread);
            if (maxdp < clients[i].fd + 1)
                maxdp = clients[i].fd + 1;
        }

        int ret = select(maxdp, &fdread, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(listener, &fdread)) {
            int client = accept(listener, NULL, NULL);
            if (nClients < MAX_CLIENTS) {
                clients[nClients].fd = client;
                clients[nClients].state = 0;
                clients[nClients].user[0] = 0;
                clients[nClients].pass[0] = 0;
                nClients++;

                printf("New client connected: %d\n", client);
                char *msg = "User: ";
                send(client, msg, strlen(msg), 0);
            } else {
                char *msg = "Sorry. Out of slots.\n";
                send(client, msg, strlen(msg), 0);
                close(client);
            }
        }

        for (int i = 0; i < nClients; i++) {
            if (FD_ISSET(clients[i].fd, &fdread)) {
                ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    printf("Client %d disconnected\n", clients[i].fd);
                    removeClient(clients, &nClients, i);
                    i--;
                    continue;
                }

                buf[ret] = 0;
                trim(buf);

                if (clients[i].state == 0) {
                    strcpy(clients[i].user, buf);
                    clients[i].state = 1;
                    char *msg = "Pass: ";
                    send(clients[i].fd, msg, strlen(msg), 0);
                } else if (clients[i].state == 1) {
                    strcpy(clients[i].pass, buf);

                    if (checkUserPass(clients[i].user, clients[i].pass)) {
                        clients[i].state = 2;
                        char *msg = "Login successful\n";
                        send(clients[i].fd, msg, strlen(msg), 0);
                    } else {
                        char *msg = "Login failed\nUser: ";
                        clients[i].state = 0;
                        send(clients[i].fd, msg, strlen(msg), 0);
                    }
                } else {
                    char cmd[300];
                    sprintf(cmd, "%s > out.txt", buf);
                    system(cmd);
                    sendFile(clients[i].fd, "out.txt");
                }
            }
        }
    }

    close(listener);
    return 0;
}