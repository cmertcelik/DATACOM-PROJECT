// Server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <poll.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket[MAX_CLIENTS];
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    int scores[MAX_CLIENTS] = {0};
    struct pollfd fds[MAX_CLIENTS];

    // Soket aç
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // soketle portu bağla
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 2 kişi bağlanana kadar bağlantıları dinle
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for clients to connect...\n");

    // bağlantılar için soket aç
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((new_socket[i] = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        printf("Client %d connected.\n", i + 1);

        // oyuncudan isim al
        send(new_socket[i], "Enter your name: ", strlen("Enter your name: "), 0);
        read(new_socket[i], buffer, BUFFER_SIZE);
        buffer[strcspn(buffer, "\n")] = 0;
        printf("Client %d name: %s\n", i + 1, buffer);

        fds[i].fd = new_socket[i];
        fds[i].events = POLLIN;
    }

    printf("Both clients are connected. Starting the game.\n");

    
    while (1) {
        srand(time(0)); //sayı üret
        int secret_number = rand() % 10 + 1;
        printf("Secret number: %d\n", secret_number);

        int guesses[MAX_CLIENTS] = {0};
        int ready_clients = 0;

        // oyuncudan tahminleri al
        for (int i = 0; i < MAX_CLIENTS; i++) {
            send(new_socket[i], "Enter your guess : ", strlen("Enter your guess : "), 0);
        }

        // tüm cevaplar gelene kadar bekle
        while (ready_clients < MAX_CLIENTS) {
            int poll_count = poll(fds, MAX_CLIENTS, -1);

            if (poll_count > 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (fds[i].revents & POLLIN) {
                        memset(buffer, 0, BUFFER_SIZE);
                        read(new_socket[i], buffer, BUFFER_SIZE);
                        guesses[i] = atoi(buffer);
                        ready_clients++;
                        fds[i].revents = 0;
                    }
                }
            }
        }

        int diff1 = abs(secret_number - guesses[0]);
        int diff2 = abs(secret_number - guesses[1]);

        if (diff1 < diff2) {
            scores[0]++;
            printf("oyuncu 1 bildi!\n");
        } else if (diff2 < diff1) {
            scores[1]++;
            printf("oyuncu 2 bildi!\n");
        } else {
            printf("puan alan yok.\n");
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            snprintf(buffer, BUFFER_SIZE, "guncel puan: %d\n", scores[i]);
            send(new_socket[i], buffer, strlen(buffer), 0);
        }

        if (scores[0] >= 10) {
            send(new_socket[0], "kazandin!\n", strlen("kazandin!\n"), 0);
            send(new_socket[1], "kaybettin.\n", strlen("kaybettin.\n"), 0);
            break;
        } else if (scores[1] >= 10) {
            send(new_socket[1], "kazandin!\n", strlen("kazandin!\n"), 0);
            send(new_socket[0], "kaybettin.\n", strlen("kaybettin.\n"), 0);
            break;
        }

        ready_clients = 0;
    }

    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        close(new_socket[i]); //soketleri kapa
    }
    close(server_fd); //sunucuyu kapa

    return 0;
}
 