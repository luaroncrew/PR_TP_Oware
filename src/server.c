#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "commands.h"

#define PORT 8083
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024



// Array to hold connected clients
client_t* clients[MAX_CLIENTS];

// Function to handle client communication
void* handle_client(void* arg) {
    client_t* client = (client_t*)arg;
    char buffer[BUFFER_SIZE] = {0};

    send(client->socket_fd, "available commands: bla bla bla \n", 40, 0);


    // Communication loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer

        // Read data from the client
        int valread = recv(client->socket_fd, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) {
            printf("Client %s disconnected\n", client->username);
            break; // Exit the loop if the client has disconnected
        }

        // Process commands (for now, just print them)
        printf("%s: %s\n", client->username, buffer);

        // Handle commands here (for example):
        if (strncmp(buffer, "/exit", 5) == 0) {
            send(client->socket_fd, "quit\n", 9, 0);
            break; // Break the loop for exit command
        } else if (strncmp(buffer, "/message", 8) == 0) {
            // Placeholder for message functionality
            send(client->socket_fd, "Message command received.\n", 26, 0);
        }
        else if (strncmp(buffer, "/see_users", 10) == 0) {
            see_users(client);
        }
        else if (strncmp(buffer, "/login", 6) == 0){
            login_procedure(client);
        }
        else if (strncmp(buffer, "/game", 5) == 0) {
            send_game_request(client);
        }
        else if (strncmp(buffer, "/accept", 7) == 0) {
            accept_game_request(client);
        }
        else if (strncmp(buffer, "/help", 5) == 0){
            send(client->socket_fd, "available commands: bla bla bla \n", 40, 0);
        }
        else if (strncmp(buffer, "/join", 5) == 0){
            join_game(client);
            break;
        }
        else {
            printf("unrecognized command: %s", buffer);
            send(client->socket_fd, "Unknown command.\n", 17, 0);
        }
    }

    // Clean up and close the client socket
    close(client->socket_fd);
    free(client); // Free the allocated memory for the client
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t threads[MAX_CLIENTS]; // Array to hold thread IDs
    int thread_count = 0;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Load all saved games at startup
    load_saved_games();

    printf("Server is listening on port %d...\n", PORT);

    // Accept connections in a loop
    while (thread_count < MAX_CLIENTS) {
        client_t* new_client = malloc(sizeof(client_t)); // Allocate memory for new client
        if ((new_socket = accept(server_fd, (struct sockaddr*)&new_client->address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            free(new_client); // Free memory if accept fails
            continue;
        }

        new_client->socket_fd = new_socket; // Set the socket fd in the client struct
        clients[thread_count] = new_client; // Add new client to the array
        printf("New connection accepted\n");



        // Create a new thread for the client
        if (pthread_create(&threads[thread_count], NULL, handle_client, (void*)new_client) != 0) {
            perror("Failed to create thread");
            free(new_client); // Free memory if thread creation fails
            continue;
        }

        thread_count++; // Increment the thread count
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }

    // Close the server socket
    close(server_fd);
    return 0;
}
