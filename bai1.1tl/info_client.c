#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    if (argc!=3)
    {
            printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }
    int client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    connect(client, (struct sockaddr*)&addr, sizeof(addr));

    char buffer[1024];
    // lay file
    FILE *fp = popen("pwd", "r");
    fgets(buffer, sizeof(buffer), fp);
    send(client, buffer, strlen(buffer), 0);
    pclose(fp);
    // lay danh sach file
    fp = popen("ls -l | awk '{print $9 \" - \" $5 \" bytes\"}'", "r");
    while (fgets(buffer, sizeof(buffer), fp)) {
        send(client, buffer, strlen(buffer), 0);
    }
    pclose(fp);

    close(client);
}