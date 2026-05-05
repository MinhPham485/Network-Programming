#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_CLIENT 100
#define BUF_SIZE 1024

typedef struct {
    int socket;
    char topics[10][50];
    int topic_count;
} Client;

Client clients[MAX_CLIENT];
int client_count = 0;

// kiểm tra client có subscribe topic chưa
int is_subscribed(Client *c, char *topic) {
    for (int i = 0; i < c->topic_count; i++) {
        if (strcmp(c->topics[i], topic) == 0)
            return 1;
    }
    return 0;
}

// thêm topic
void add_topic(Client *c, char *topic) {
    if (!is_subscribed(c, topic)) {
        strcpy(c->topics[c->topic_count++], topic);
    }
}

// xóa topic
void remove_topic(Client *c, char *topic) {
    for (int i = 0; i < c->topic_count; i++) {
        if (strcmp(c->topics[i], topic) == 0) {
            for (int j = i; j < c->topic_count - 1; j++) {
                strcpy(c->topics[j], c->topics[j+1]);
            }
            c->topic_count--;
            break;
        }
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);

        int maxfd = listener;

        for (int i = 0; i < client_count; i++) {
            FD_SET(clients[i].socket, &readfds);
            if (clients[i].socket > maxfd)
                maxfd = clients[i].socket;
        }

        select(maxfd + 1, &readfds, NULL, NULL, NULL);

        // có client mới
        if (FD_ISSET(listener, &readfds)) {
            int client = accept(listener, NULL, NULL);
            clients[client_count].socket = client;
            clients[client_count].topic_count = 0;
            client_count++;
            printf("New client connected\n");
        }

        // xử lý client
        for (int i = 0; i < client_count; i++) {
            if (FD_ISSET(clients[i].socket, &readfds)) {
                char buf[BUF_SIZE];
                int n = recv(clients[i].socket, buf, sizeof(buf)-1, 0);

                if (n <= 0) {
                    close(clients[i].socket);
                    clients[i] = clients[--client_count];
                    i--;
                    continue;
                }

                buf[n] = 0;

                char cmd[10], topic[50], msg[500];

                sscanf(buf, "%s %s %[^\n]", cmd, topic, msg);

                if (strcmp(cmd, "SUB") == 0) {
                    add_topic(&clients[i], topic);
                    printf("Client SUB %s\n", topic);
                }

                else if (strcmp(cmd, "UNSUB") == 0) {
                    remove_topic(&clients[i], topic);
                    printf("Client UNSUB %s\n", topic);
                }

                else if (strcmp(cmd, "PUB") == 0) {
                    printf("Publish %s: %s\n", topic, msg);

                    for (int j = 0; j < client_count; j++) {
                        if (is_subscribed(&clients[j], topic)) {
                            send(clients[j].socket, msg, strlen(msg), 0);
                        }
                    }
                }
            }
        }
    }
}