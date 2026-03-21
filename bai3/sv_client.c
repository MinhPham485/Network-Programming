#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        return 1;
    }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    connect(client, (struct sockaddr*)&addr, sizeof(addr));

    char mssv[50], name[100], dob[50], gpa[50];
    char msg[256];

    printf("MSSV: ");
    fgets(mssv, sizeof(mssv), stdin);
    mssv[strcspn(mssv, "\n")] = 0;

    printf("Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("DOB: ");
    fgets(dob, sizeof(dob), stdin);
    dob[strcspn(dob, "\n")] = 0;

    printf("GPA: ");
    fgets(gpa, sizeof(gpa), stdin);
    gpa[strcspn(gpa, "\n")] = 0;
    sprintf(msg, "%s %s %s %s", mssv, name, dob, gpa);

    send(client, msg, strlen(msg), 0);

    close(client);
    return 0;
}