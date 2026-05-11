#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

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

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    char buf[256];
    char *ids[FD_SETSIZE] = {NULL};

    while (1) {
        fdtest = fdread;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select() failed");
            break;
        }

        for (int i = 0; i < FD_SETSIZE; i++) {
            if (!FD_ISSET(i, &fdtest)) continue;

            if (i == listener) {
                int client = accept(listener, NULL, NULL);
                if (client < FD_SETSIZE) {
                    printf("New client connected %d\n", client);
                    FD_SET(client, &fdread);
                    char *msg = "Nhap theo cu phap: client_id: client_name\n";
                    send(client, msg, strlen(msg), 0);
                } else {
                    close(client);
                }
            } else {
                ret = recv(i, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    printf("Client %d disconnected\n", i);
                    FD_CLR(i, &fdread);
                    if (ids[i] != NULL) {
                        free(ids[i]);
                        ids[i] = NULL;
                    }
                    close(i);
                    continue;
                }

                buf[ret] = 0;
                if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;
                if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = 0;

                printf("Received from %d: %s\n", i, buf);

                if (ids[i] == NULL) {
                    char id[32], name[32], extra[32];
                    int n = sscanf(buf, "%31[^:]: %31s %31s", id, name, extra);

                    if (n != 2) {
                        char *msg = "Sai cu phap. Nhap lai: client_id: client_name\n";
                        send(i, msg, strlen(msg), 0);
                    } else {
                        ids[i] = malloc(strlen(id) + 1);
                        strcpy(ids[i], id);

                        char *msg = "OK. Hay nhap tin nhan!\n";
                        send(i, msg, strlen(msg), 0);
                    }
                } else {
                    char out[512];
                    sprintf(out, "%s: %s\n", ids[i], buf);

                    for (int j = 0; j < FD_SETSIZE; j++) {
                        if (j != i && ids[j] != NULL) {
                            send(j, out, strlen(out), 0);
                        }
                    }
                }
            }
        }
    }

    close(listener);
    return 0;
}