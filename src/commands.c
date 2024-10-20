#include <stdio.h>
#include <string.h>
#include <ctype.h>


#include "commands.h"
#include "game.h"

#define MAX_GAME_REQUESTS 10
#define MAX_GAMES 10

#define MAX_SAVED_GAMES 10

saved_game_t saved_games[MAX_SAVED_GAMES];
int saved_game_count = 0;

typedef struct {
    client_t* challenger;  // Pointer to the challenger client
    client_t* challenged; // Pointer to the challenged client
    int accepted; // Flag to indicate if the request has been accepted
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


// Function to trim trailing spaces from a string
void trim_trailing_spaces(char* str) {
    int n = strlen(str);
    while (n > 0 && isspace((unsigned char)str[n - 1])) {
        n--;
    }
    str[n] = '\0';
}

// Modified login_procedure function
void login_procedure(client_t* client) {
    // Prompt for username
    send(client->socket_fd, "Please enter your username: \n", 36, 0);
    // Create a temporary variable to store the username
    char username[50] = {0}; // Initialize with zeros

    // Read the username from the client
    int bytes_received = recv(client->socket_fd, username, sizeof(username) - 1, 0); // Leave space for null-terminator
    if (bytes_received < 0) {
        perror("recv failed");
        return;
    }
    username[bytes_received] = '\0'; // Ensure null-termination

    // Trim trailing spaces from the entered username
    trim_trailing_spaces(username);

    // Ensure the username is not taken
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            continue;
        }

        // Trim trailing spaces from the existing username
        char trimmed_client_username[50];
        strncpy(trimmed_client_username, clients[i]->username, sizeof(trimmed_client_username) - 1);
        trimmed_client_username[sizeof(trimmed_client_username) - 1] = '\0'; // Ensure null-termination
        trim_trailing_spaces(trimmed_client_username);

        printf("Usernames to compare: '%s' and '%s'\n", trimmed_client_username, username);

        // Compare the trimmed usernames
        if (strcmp(trimmed_client_username, username) == 0) {
            send(client->socket_fd, "Username already taken. Please try again.\n", 42, 0);
            username[0] = '\0'; // Reset the username
            return;
        }
    }

    // Assign the username to the client
    strncpy(client->username, username, sizeof(client->username) - 1);
    client->username[sizeof(client->username) - 1] = '\0'; // Ensure null-termination

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
    // Mark the request as accepted
    game_requests[selected_request_index].accepted = 1;

    // Remove the game request from the list
    for (int i = selected_request_index; i < game_request_count - 1; i++) {
        game_requests[i] = game_requests[i + 1]; // Shift left
    }
    game_request_count--;

    play_game(new_game, client); // Start the game
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
        send(client->socket_fd, "Invalid selection. You're back in the main menu \n", 19, 0);
        return;
    }

    // Find the selected user
    int selected_index = available_users[selection - 1];
    client_t* opponent = clients[selected_index];


    // Store the game request
    game_requests[game_request_count].challenger = client;
    game_requests[game_request_count].challenged = opponent;
    // Notify both the challenger and the opponent
    game_request_count++;

    // construct the notification message for the challenged person
    send(opponent->socket_fd, "You have received a game request.\n", 33, 0);

    // Notify the challenger
    send(client->socket_fd, "Game request sent.\n", 20, 0);
    return;
}

void join_game(client_t * client) {
    game_t * client_game;

    // find the game where the client is the challenger
    for (int i = 0; i < current_game_count; i++) {
        if (current_games[i]->player1 == client) {
            client_game = current_games[i];
            break;
        }
    }

    // if no game found, go to the main menu
    if (client_game == NULL) {
        send(client->socket_fd, "You are not in a game.\n", 23, 0);
        return;
    }

    play_game(client_game, client);
}

void disconnect(client_t* client, char* buffer) {
    // Identifier la partie de ce client et sauvegarder l'état
    for (int i = 0; i < current_game_count; i++) {
        game_t* game = current_games[i];
        if (game->player1 == client || game->player2 == client) {
            save_game_state(game, game->player1, game->player2, game->status);
            printf("Game saved upon %s's disconnection.\n", client->username);
            break;
        }
    }

    // Libérer les ressources du client
    printf("Client %s disconnected.\n", client->username);
    close(client->socket_fd);
    free(client);
}

void reconnect(client_t* client) {
    for (int i = 0; i < saved_game_count; i++) {
        saved_game_t* saved_game = &saved_games[i];
        if (strcmp(saved_game->player1->username, client->username) == 0 ||
            strcmp(saved_game->player2->username, client->username) == 0) {
            printf("Resuming saved game for %s.\n", client->username);
            play_game(saved_game->game, client);  // Reprendre la partie
            break;
        }
    }
}

void save_game_state(game_t* game, client_t* player1, client_t* player2, int current_turn) {
    if (saved_game_count >= MAX_SAVED_GAMES) {
        printf("Maximum number of saved games reached.\n");
        return;
    }

    saved_game_t* saved_game = &saved_games[saved_game_count++];
    saved_game->game = game;
    saved_game->player1 = player1;
    saved_game->player2 = player2;
    saved_game->player_turn = current_turn;

    printf("Game between %s and %s has been saved.\n", player1->username, player2->username);
}

game_t* load_saved_game(client_t* player1, client_t* player2) {
    for (int i = 0; i < saved_game_count; i++) {
        if ((saved_games[i].player1 == player1 && saved_games[i].player2 == player2) ||
            (saved_games[i].player1 == player2 && saved_games[i].player2 == player1)) {
            printf("Resuming game between %s and %s.\n", player1->username, player2->username);
            return saved_games[i].game;
        }
    }

    printf("No saved game found for %s and %s.\n", player1->username, player2->username);
    return NULL;
}

void list_saved_opponents(client_t* client) {
    int opponent_count = 0;

    send(client->socket_fd, "Select an opponent to reload the game:\n", 40, 0);
    
    for (int i = 0; i < saved_game_count; i++) {
        if (saved_games[i].player1 == client || saved_games[i].player2 == client) {
            client_t* opponent = (saved_games[i].player1 == client) ? saved_games[i].player2 : saved_games[i].player1;
            char message[BUFFER_SIZE];
            snprintf(message, BUFFER_SIZE, "%d. %s\n", opponent_count + 1, opponent->username);
            send(client->socket_fd, message, strlen(message), 0);
            opponent_count++;
        }
    }

    if (opponent_count == 0) {
        send(client->socket_fd, "No saved games found.\n", 22, 0);
    }
}

client_t* get_opponent_by_choice(client_t* client, int choice) {
    int opponent_count = 0;

    for (int i = 0; i < saved_game_count; i++) {
        if (saved_games[i].player1 == client || saved_games[i].player2 == client) {
            client_t* opponent = (saved_games[i].player1 == client) ? saved_games[i].player2 : saved_games[i].player1;
            if (opponent_count + 1 == choice) {
                return opponent;
            }
            opponent_count++;
        }
    }

    return NULL;
}



int is_logged_in(client_t* client) {
    return client->username[0] != '\0';
}

