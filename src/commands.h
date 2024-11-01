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
void wait_for_opponent(game_t* saved_game, client_t* player);
bool is_user_connected(client_t* user);
void load_saved_games();


#endif // COMMANDS_H
