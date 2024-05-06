#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_PORT 50535
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_address;
    char message_buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Main loop to continuously send and receive messages until "exit" command
    while (1) {
        printf("Enter message: ");
        fgets(message_buffer, BUFFER_SIZE, stdin);

        // Send message to server
        send(client_socket, message_buffer, strlen(message_buffer), 0);

        // Exit if message is "exit"
        if (strcmp(message_buffer, "exit\n") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Receive response from server
        memset(message_buffer, 0, sizeof(message_buffer));
        read(client_socket, message_buffer, BUFFER_SIZE);
        printf("Server response: %s\n", message_buffer);
    }

    // Close client socket
    close(client_socket);
    return 0;
}
