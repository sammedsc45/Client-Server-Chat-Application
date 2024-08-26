#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 2048
#define NICKNAME_LEN 32

void handle_server_response(int socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        printf("%s", buffer);
    }

    if (bytes_received == 0) {
        printf("Server disconnected.\n");
    } else {
        perror("recv failed");
    }

    close(socket);
    exit(EXIT_SUCCESS);
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char server_ip[INET_ADDRSTRLEN];
    int server_port;
    char nickname[NICKNAME_LEN];
    char message[BUFFER_SIZE];
    fd_set read_fds;
    struct timeval timeout;

    // Get server details from user
    printf("Enter server IP address: ");
    scanf("%s", server_ip);
    printf("Enter server port number: ");
    scanf("%d", &server_port);

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Get nickname
    printf("Connected to server. Please enter a nickname: ");
    scanf("%s", nickname);
    send(client_socket, nickname, strlen(nickname), 0);

    // Create a thread to handle server responses
    if (fork() == 0) {
        handle_server_response(client_socket);
    }

    // Main loop for user input
    while (1) {
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character

        if (send(client_socket, message, strlen(message), 0) < 0) {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        if (strcmp(message, "/quit") == 0) {
            break;
        }
    }

    close(client_socket);
    return 0;
}
