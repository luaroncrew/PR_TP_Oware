#ifndef COMMANDS_H
#define COMMANDS_H

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>


#include "server.h" // Include the server header to access the client_t structure
#include "game.h"

void see_users(client_t* client);
void login_procedure(client_t* client);
void send_game_request(client_t* client);
void accept_game_request(client_t* client);
void join_game(client_t* client);
void load_saved_games();
int is_logged_in(client_t* client);


#endif // COMMANDS_H
