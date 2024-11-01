#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8083
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_GAMES 100

typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    char username[50];
} client_t;

// Array to hold connected clients
extern client_t* clients[MAX_CLIENTS]; // Use 'extern' to indicate it's defined elsewhere

#endif // SERVER_H
