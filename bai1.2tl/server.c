#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(listener, (struct sockaddr*)&addr, sizeof(addr));
    listen(listener, 5);

    printf("Server listening...\n");

    int client = accept(listener, NULL, NULL);

    char buf[1024];
    char total[5000] = "";  
    int count = 0;
    int n;

    while ((n = recv(client, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = 0;
        printf("Received: %s\n", buf);

        strcat(total, buf);

        char *p = total;
        while ((p = strstr(p, "0123456789")) != NULL) {
            memmove(total, p + 10, strlen(p + 10) + 1); // xoa du lieu da xu ly
            count++;
            p += 10; 
        }

        printf("Count = %d\n", count);
        //tranh buffer lon
        if (strlen(total) > 4000) {
            strcpy(total, total + 2000);
        }
    }

    close(client);
    close(listener);
}