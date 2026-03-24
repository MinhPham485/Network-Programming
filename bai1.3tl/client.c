#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int main() {
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8000);

    char buf[256];

    while (1) {
        printf("Enter string: ");
        fgets(buf, sizeof(buf), stdin);

        sendto(sender, buf, strlen(buf), 0,
            (struct sockaddr *)&addr, sizeof(addr));

        // 🔥 nhận lại từ server
        int n = recvfrom(sender, buf, sizeof(buf)-1, 0, NULL, NULL);
        buf[n] = 0;

        printf("Echo: %s\n", buf);
    }
}