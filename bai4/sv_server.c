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

    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listener);
        return 1;
    }

    // Listen
    if (listen(listener, 5) < 0) {
        perror("listen");
        close(listener);
        return 1;
    }

    printf("Server running on port %d...\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr); 

        int client = accept(listener, (struct sockaddr*)&client_addr, &len);
        if (client < 0) {
            perror("accept");
            continue;  
        }

        char buf[256];
        int n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';

            // take ip client
            char *ip = inet_ntoa(client_addr.sin_addr);

            // take time
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
            } else {
                perror("fopen");
            }
        } else if (n < 0) {
            perror("recv");
        }

        close(client);  
    }

    close(listener);
    return 0;
}