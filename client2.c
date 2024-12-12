// Client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char input[BUFFER_SIZE] = {0};

    // soket aç
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // adresi dönüştür
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return -1;
    }

    // servera bağlan
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server.\n");

    // oyundan gelen sorular ve cevap gönderme
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        if (read(sock, buffer, BUFFER_SIZE) <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        printf("%s", buffer);

        
        memset(input, 0, BUFFER_SIZE);
        fgets(input, BUFFER_SIZE, stdin);
        input[strcspn(input, "\n")] = 0;

        // Sunucuya gönder
        send(sock, input, strlen(input), 0);
    }

    // soketi kapat
    close(sock);
    return 0;
}
