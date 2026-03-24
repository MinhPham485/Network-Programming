#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <PORT> <LOG_FILE>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *logfile = argv[2];

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server running...\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);  

        int client = accept(listener, (struct sockaddr*)&client_addr, &len);

        char buf[256];
        int n = recv(client, buf, sizeof(buf)-1, 0);

        if (n > 0) {
            buf[n] = 0;

            // IP
            char *ip = inet_ntoa(client_addr.sin_addr);

            // Time
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char time_str[64];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

            printf("%s %s %s\n", ip, time_str, buf);

            // file
            FILE *f = fopen(logfile, "a");
            if (f != NULL) {
                fprintf(f, "%s %s %s\n", ip, time_str, buf);
                fclose(f);
            }
        }

        close(client);
    }
}