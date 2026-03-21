#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <PORT> <WELCOME_FILE> <OUTPUT_FILE>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *welcome_file = argv[2];
    char *output_file = argv[3];

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server dang cho ket noi...\n");

    int client = accept(listener, NULL, NULL);
    printf("Client connected\n");

    
    FILE *f = fopen(welcome_file, "r");
    if (f != NULL) {
        char msg[256];
        fgets(msg, sizeof(msg), f);
        send(client, msg, strlen(msg), 0);
        fclose(f);
    }

    FILE *out = fopen(output_file, "a");
    char buf[256];
    int n;

    while ((n = recv(client, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = 0;
        printf("Nhan: %s", buf);

        if (out != NULL) {
            fputs(buf, out);
            fflush(out);
        }
    }

    if (out != NULL) fclose(out);

    close(client);
    close(listener);
    return 0;
}