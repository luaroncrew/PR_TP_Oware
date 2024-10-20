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

#include "game.h"  // Inclusion du fichier qui contient la définition de game_t

typedef struct {
    game_t* game;
    client_t* player1;
    client_t* player2;
    int player_turn;    // First player to play
} saved_game_t;


void see_users(client_t* client);
void login_procedure(client_t* client);
void send_game_request(client_t* client);
void accept_game_request(client_t* client);
void join_game(client_t* client);
void disconnect(client_t* client, char* buffer);
void save_game_state(game_t* game, client_t* player1, client_t* player2, int current_turn);
void reconnect(client_t* client);
game_t* load_saved_game(client_t* player1, client_t* player2);
void list_saved_opponents(client_t* client);
client_t* get_opponent_by_choice(client_t* client, int choice);

#endif // COMMANDS_H
