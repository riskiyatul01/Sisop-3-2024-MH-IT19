#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <curl/curl.h>

#define PORT 50535
#define BUFFER_SIZE 1024
#define LOG_FILE "change.log"

// Function to download file using libcurl
int download_file(const char *url, const char *filename) {
    CURL *curl;
    FILE *fp;
    CURLcode res;
    curl = curl_easy_init();
    if (curl) {
        fp = fopen(filename, "wb");
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to perform request: %s\n", curl_easy_strerror(res));
            fclose(fp);
            curl_easy_cleanup(curl);
            return 0;
        }
        fclose(fp);
        curl_easy_cleanup(curl);
        return 1;
    }
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int valread;

    // Read client message
    valread = read(client_socket, buffer, BUFFER_SIZE);
    printf("Client message: %s\n", buffer);
    fflush(stdout); // Force writing output to terminal

    // Process client message
    if (strcmp(buffer, "exit\n") == 0) {
        printf("Closing connection with client\n");
        close(client_socket);
        return;
    } else if (strcmp(buffer, "download\n") == 0) {
        // Download file using libcurl
        char *url = "https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50";
        char *filename = "myanimelist.csv";
        int success = download_file(url, filename);
        if (success) {
            printf("File downloaded successfully\n");
            // Respond to client
            char *response = "File downloaded successfully";
            send(client_socket, response, strlen(response), 0);
        } else {
            fprintf(stderr, "Failed to download file\n");
            // Respond to client
            char *response = "Failed to download file";
            send(client_socket, response, strlen(response), 0);
        }
    } else {
        // Respond to client
        char *response = "Message received";
        send(client_socket, response, strlen(response), 0);
    }

    // Log the client message
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        time_t current_time;
        time(&current_time);
        fprintf(log_file, "[%s] %s\n", asctime(localtime(&current_time)), buffer);
        fclose(log_file);
    } else {
        printf("Failed to open log file\n");
        fflush(stdout); // Force writing output to terminal
    }
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind server socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    fflush(stdout); // Force writing output to terminal

    // Accept client connections and handle messages
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        handle_client(client_socket);
    }

    return 0;
}

