#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "game.h"

#define MAX_GAME_REQUESTS 10
#define MAX_GAMES 10


typedef struct {
    client_t* challenger;  // Pointer to the challenger client
    client_t* challenged;   // Pointer to the challenged client
} game_request_t;


game_request_t game_requests[MAX_GAME_REQUESTS]; // Array to hold game requests
int game_request_count = 0; // Number of current game requests

game_t * current_games[MAX_GAMES]; // Array to hold current games
int current_game_count = 0; // Number of current games


// Function to display the list of connected users
void see_users(client_t* client) {
    char user_list[BUFFER_SIZE] = "Connected users:\n";

    // Loop through the client array to add usernames to the list
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL) {
            strcat(user_list, clients[i]->username);
            strcat(user_list, "\n");
        }
    }

    // Send the list of connected users to the requesting client
    send(client->socket_fd, user_list, strlen(user_list), 0);
}

// log in user
void login_procedure(client_t* client) {
    // Prompt for username
    send(client->socket_fd, "Please enter your username: \n", 36, 0);
    // Receive the username from client
    recv(client->socket_fd, client->username, sizeof(client->username), 0);
    // Ensure the username is null-terminated to avoid issues
    client->username[strcspn(client->username, "\n")] = 0;  // Removing newline character, if any
    // Construct the welcome message
    char welcome_message[BUFFER_SIZE];
    snprintf(welcome_message, sizeof(welcome_message), "Welcome, %s!\n", client->username);
    // Send the personalized welcome message to the client
    send(client->socket_fd, welcome_message, strlen(welcome_message), 0);
    // Log the connection on the server side
    printf("User connected: %s\n", client->username);
}


void accept_game_request(client_t* client) {
    char response[BUFFER_SIZE];

    // Notify the user about the game requests addressed to them
    snprintf(response, sizeof(response), "Game Requests:\n");

    int request_index = 1; // Counter for user-friendly numbering
    int valid_requests[MAX_GAME_REQUESTS]; // To store the indexes of valid requests
    int valid_request_count = 0;

    // Iterate through the game requests to find those addressed to the client
    for (int i = 0; i < game_request_count; i++) {
        printf(
                "Challenger: %s, Challenged: %s\n",
                game_requests[i].challenger->username,
                game_requests[i].challenged->username
                );
        if (strcmp(game_requests[i].challenged->username, client->username) == 0) {
            snprintf(response + strlen(response), sizeof(response) - strlen(response),
                     "%d - Game with %s\n", request_index, game_requests[i].challenger->username);
            valid_requests[valid_request_count++] = i; // Store the valid request index
            request_index++;
        }
    }

    if (request_index == 1) {
        send(client->socket_fd, "You have no game requests.\n", 28, 0);
        return;
    } else {
        send(client->socket_fd, response, strlen(response), 0);
    }

    // Prompt the user to accept a game request
    send(client->socket_fd, "Please enter the request number to accept: ", 42, 0);
    char buffer[10]; // Buffer to hold the user's response
    recv(client->socket_fd, buffer, sizeof(buffer), 0);
    int request_number = atoi(buffer);

    // Check if the request number is valid
    if (request_number < 1 || request_number > valid_request_count) {
        send(client->socket_fd, "Invalid request number.\n", 24, 0);
        return;
    }

    int selected_request_index = valid_requests[request_number - 1];
    game_request_t selected_request = game_requests[selected_request_index];

    // Find the challenger client
    client_t* challenger = selected_request.challenger;
    if (challenger == NULL) {
        send(client->socket_fd, "Challenger not found, unable to join the game.\n", 46, 0);
        return;
    }

    // Ensure the server has room for more games
    if (current_game_count >= MAX_GAMES) {
        send(client->socket_fd, "Server cannot start more games, try again later.\n", 49, 0);
        return;
    }

    // Create the new game instance
    game_t* new_game = (game_t*)malloc(sizeof(game_t));
    if (new_game == NULL) {
        perror("Failed to allocate memory for game");
        return;
    }

    // Initialize the game
    new_game->player1 = challenger;
    new_game->player2 = client;
    new_game->player_scores[PLAYER1] = 0;
    new_game->player_scores[PLAYER2] = 0;
    new_game->status = PLAYER1_TURN;
    initialize_board(new_game);

    // Add the new game to the server's current_games array
    current_games[current_game_count++] = new_game;

    // Remove the game request from the list
    for (int i = selected_request_index; i < game_request_count - 1; i++) {
        game_requests[i] = game_requests[i + 1]; // Shift left
    }
    game_request_count--;

    play_game(new_game); // Start the game
    return;
}



void send_game_request(client_t* client) {
    char buffer[BUFFER_SIZE] = {0};
    char user_list[BUFFER_SIZE] = "Choose a user to challenge:\n";

    // List the connected users in numbered format
    int available_users[MAX_CLIENTS]; // Array to store indexes of valid users
    int count = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket_fd != client->socket_fd) {
            available_users[count++] = i; // Store the index of available users
            char line[50];
            snprintf(line, sizeof(line), "%d - %s\n", count, clients[i]->username);
            strcat(user_list, line);
        }
    }

    if (count == 0) {
        send(client->socket_fd, "No other users to challenge.\n", 30, 0);
        return;
    }

    // Send the list of users to the challenger
    send(client->socket_fd, user_list, strlen(user_list), 0);

    // Prompt the client for input
    send(client->socket_fd, "Enter the number of the user you want to challenge:\n", 52, 0);

    // Read the user's selection (blocking until a valid input)
    int valread = recv(client->socket_fd, buffer, BUFFER_SIZE, 0);
    if (valread <= 0) {
        send(client->socket_fd, "Failed to receive input.\n", 25, 0);
        return;
    }

    int selection = atoi(buffer); // Convert the input to an integer
    if (selection < 1 || selection > count) {
        send(client->socket_fd, "Invalid selection.\n", 19, 0);
        return;
    }

    // Find the selected user
    int selected_index = available_users[selection - 1];
    client_t* opponent = clients[selected_index];

    printf("opponent : %s, current client: %s", opponent->username, client->username);

    // Store the game request
    game_requests[game_request_count].challenger = client;
    game_requests[game_request_count].challenged = opponent;
    // Notify both the challenger and the opponent
    game_request_count++;
    send(client->socket_fd, "Game request sent.\n", 20, 0);
    return;

}


