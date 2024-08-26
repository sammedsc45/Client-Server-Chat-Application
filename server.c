#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 5001
#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define NICKNAME_LEN 32

typedef struct {
    struct sockaddr_in address;
    int socket;
    int index;
    char nickname[NICKNAME_LEN];
} client_t;

client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (!clients[i]) {
            clients[i] = client;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int index) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && clients[i]->index == index) {
            clients[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

int is_nickname_taken(const char *nickname) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && strcmp(clients[i]->nickname, nickname) == 0) {
            pthread_mutex_unlock(&clients_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return 0;
}

void broadcast_message(char *message, int sender_index) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && clients[i]->index != sender_index) {
            if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                perror("Broadcast failed");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    printf("Broadcast message sent: %s", message);
}

void send_private_message(char *message, char *nickname) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i] && strcmp(clients[i]->nickname, nickname) == 0) {
            if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                perror("Private message failed");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void list_clients(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    char list[BUFFER_SIZE] = "Connected clients:\n";
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i]) {
            char client_info[100];
            snprintf(client_info, sizeof(client_info), "- %s (%s:%d)\n", clients[i]->nickname, inet_ntoa(clients[i]->address.sin_addr), ntohs(clients[i]->address.sin_port));
            strcat(list, client_info);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    send(client_socket, list, strlen(list), 0);
}

void send_help(int client_socket) {
    const char *help_message =
        "Available commands:\n"
        "/list - List connected clients\n"
        "/private <nickname> <message> - Send a private message\n"
        "/broadcast <message> - Send a broadcast message\n"
        "/quit - Quit the chat\n";
    send(client_socket, help_message, strlen(help_message), 0);
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Receive nickname
    if ((bytes_received = recv(client->socket, client->nickname, NICKNAME_LEN, 0)) <= 0) {
        perror("Receive nickname failed");
        close(client->socket);
        free(client);
        return NULL;
    }
    client->nickname[bytes_received] = '\0';

    // Check for duplicate nickname
    if (is_nickname_taken(client->nickname)) {
        const char *error_message = "Nickname already taken. Disconnecting.\n";
        send(client->socket, error_message, strlen(error_message), 0);
        close(client->socket);
        free(client);
        return NULL;
    }

    // Add client to list
    add_client(client);

    printf("%s has connected.\n", client->nickname);

    // Send welcome message to the client
    char welcome_message[BUFFER_SIZE];
    snprintf(welcome_message, sizeof(welcome_message), "Welcome, %s! Type '/help' for a list of commands.\n", client->nickname);
    send(client->socket, welcome_message, strlen(welcome_message), 0);

    // Handle client messages
    while ((bytes_received = recv(client->socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "/list", 5) == 0) {
            list_clients(client->socket);
        } else if (strncmp(buffer, "/private", 8) == 0) {
            char *nickname = strtok(buffer + 9, " ");
            char *message = strtok(NULL, "\0");
            char private_message[BUFFER_SIZE];
            snprintf(private_message, sizeof(private_message), "[private msg from %s]: %s\n", client->nickname, message);
            send_private_message(private_message, nickname);
        } else if (strncmp(buffer, "/broadcast", 10) == 0) {
            char *message = buffer + 11;
            char full_broadcast_message[BUFFER_SIZE];
            snprintf(full_broadcast_message, sizeof(full_broadcast_message), "[%s]: %s\n", client->nickname, message);
            broadcast_message(full_broadcast_message, client->index);
        } else if (strncmp(buffer, "/help", 5) == 0) {
            send_help(client->socket);
        } else if (strncmp(buffer, "/quit", 5) == 0) {
            break;
        } else {
            const char *error_message = "Invalid command.\n";
            send(client->socket, error_message, strlen(error_message), 0);
        }
    }

    // Remove client from list and close connection
    close(client->socket);
    remove_client(client->index);
    printf("%s has disconnected.\n", client->nickname);
    free(client);
    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t tid;
    socklen_t client_len = sizeof(client_addr);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Prepare server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen on socket
    if (listen(server_socket, 3) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started. Listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connections
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Allocate and initialize client structure
        client_t *client = (client_t *)malloc(sizeof(client_t));
        client->address = client_addr;
        client->socket = client_socket;
        client->index = client_socket;

        // Create a thread to handle the new client
        if (pthread_create(&tid, NULL, handle_client, (void *)client) != 0) {
            perror("Thread creation failed");
            close(client_socket);
            free(client);
        }
    }

    close(server_socket);
    return 0;
}
