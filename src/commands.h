#ifndef COMMANDS_H
#define COMMANDS_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h" // Include the server header to access the client_t structure

void see_users(client_t* client);
void login_procedure(client_t* client);
void send_game_request(client_t* client);
void accept_game_request(client_t* client);

#endif // COMMANDS_H
