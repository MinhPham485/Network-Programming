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
    int state[FD_SETSIZE] = {0};
    char users[FD_SETSIZE][32];
    char passes[FD_SETSIZE][32];

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
                    state[client] = 0;
                    char *msg = "User: ";
                    send(client, msg, strlen(msg), 0);
                } else {
                    close(client);
                }
            } else {
                ret = recv(i, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    printf("Client %d disconnected\n", i);
                    FD_CLR(i, &fdread);
                    state[i] = 0;
                    close(i);
                    continue;
                }

                buf[ret] = 0;
                if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;
                if (strlen(buf) > 0 && buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = 0;

                printf("Received from %d: %s\n", i, buf);

                if (state[i] == 0) {
                    strcpy(users[i], buf);
                    state[i] = 1;
                    char *msg = "Pass: ";
                    send(i, msg, strlen(msg), 0);
                } else if (state[i] == 1) {
                    strcpy(passes[i], buf);

                    int found = 0;
                    char line[64], u[32], p[32];
                    FILE *f = fopen("users.txt", "r");
                    if (f != NULL) {
                        while (fgets(line, sizeof(line), f) != NULL) {
                            if (line[strlen(line) - 1] == '\n')
                                line[strlen(line) - 1] = 0;

                            if (sscanf(line, "%31s %31s", u, p) == 2) {
                                if (strcmp(users[i], u) == 0 && strcmp(passes[i], p) == 0) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        fclose(f);
                    }

                    if (found) {
                        state[i] = 2;
                        char *msg = "OK. Hay nhap lenh.\n";
                        send(i, msg, strlen(msg), 0);
                    } else {
                        state[i] = 0;
                        char *msg = "Sai username hoac password. User: ";
                        send(i, msg, strlen(msg), 0);
                    }
                } else {
                    char cmd[512];
                    sprintf(cmd, "%s > out.txt 2>&1", buf);
                    system(cmd);

                    FILE *f = fopen("out.txt", "rb");
                    if (f != NULL) {
                        while (1) {
                            int len = fread(buf, 1, sizeof(buf), f);
                            if (len <= 0) break;
                            send(i, buf, len, 0);
                        }
                        fclose(f);
                    }

                    send(i, "\nHay nhap lenh.\n", 16, 0);
                }
            }
        }
    }

    close(listener);
    return 0;
}