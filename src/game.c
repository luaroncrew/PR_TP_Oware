#include "game.h"
#include "server.h"
#include <stdio.h>
#include <string.h>

// Initialize the board with the standard setup (4 seeds per pit)
void initialize_board(game_t* game) {
    for (int i = 0; i < PITS; i++) {
        game->board[PLAYER1][i] = SEEDS;
        game->board[PLAYER2][i] = SEEDS;
    }
    game->player_scores[PLAYER1] = 0;
    game->player_scores[PLAYER2] = 0;
    game->status = PLAYER1_TURN;  // Player 1 starts
}

// Print the current state of the board for a client
void print_board(game_t* game) {
    char buffer[BUFFER_SIZE];

    // determine which player's turn it is
    client_t* current_player = (game->status == PLAYER1_TURN) ? game->player1 : game->player2;

    // notify the current player it's their turn
    send(current_player->socket_fd, "\n-------Your turn--------\n", 30, 0);


    // send the board to the player 1
    // display the part of the bord of player 2 (upper row)
    snprintf(buffer, sizeof(buffer), "\nPlayer %s pits:\n", game->player2->username);
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    for (int i = PITS - 1; i >= 0; i--) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER2][i]);
        send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    }

    // send delimiter
    snprintf(buffer, sizeof(buffer), "\n-----------------------\n");
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);

    // display the part of the board of player 1 (lower row)
    for (int i = 0; i < PITS; i++) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER1][i]);
        send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    }
    snprintf(buffer, sizeof(buffer), "\nPlayer %s pits:\n", game->player1->username);
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);


    // send the board to the player 2
    // display the part of the bord of player 1 (upper row)
    snprintf(buffer, sizeof(buffer), "\nPlayer %s pits:\n", game->player1->username);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);
    for (int i = PITS - 1; i >= 0; i--) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER1][i]);
        send(game->player2->socket_fd, buffer, strlen(buffer), 0);
    }

    // send delimiter
    snprintf(buffer, sizeof(buffer), "\n-----------------------\n");
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);

    // display the part of the board of player 2 (lower row)
    for (int i = 0; i < PITS; i++) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER2][i]);
        send(game->player2->socket_fd, buffer, strlen(buffer), 0);
    }
    snprintf(buffer, sizeof(buffer), "\nPlayer %s pits:\n", game->player2->username);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);
}

// Capture seeds after sowing
void capture_seeds(game_t* game, int player, int pit) {
    int opponent = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    while (pit >= 0 && (game->board[opponent][pit] == 2 || game->board[opponent][pit] == 3)) {
        game->player_scores[player] += game->board[opponent][pit];
        game->board[opponent][pit] = 0;
        pit--;
    }
}

// Sow the seeds from the chosen pit
void sow_seeds(game_t* game, int player, int pit) {
    int seeds = game->board[player][pit];
    game->board[player][pit] = 0;

    int current_pit = pit + 1;
    int current_player = player;

    while (seeds > 0) {
        if (current_pit >= PITS) {
            current_pit = 0;
            current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
        }

        // Skip if we are back to the original pit
        if (current_player == player && current_pit == pit) {
            current_pit++;
            continue;
        }

        game->board[current_player][current_pit]++;
        seeds--;
        current_pit++;
    }

    // Check if the last seed was sown in the opponent's pits for capturing
    if (current_player != player && current_pit > 0) {
        capture_seeds(game, player, current_pit - 1);
    }
}

// Get the player's move
int get_move(client_t* player, game_t * game) {
    char buffer[BUFFER_SIZE];
    int move;

    // prompt the player for the move
    snprintf(buffer, sizeof(buffer), "Player %s, choose a pit (1-6) or (7) to abandon and lose: ", player->username);
    send(player->socket_fd, buffer, strlen(buffer), 0);

    // Wait for player's response
    recv(player->socket_fd, buffer, sizeof(buffer), 0);
    move = atoi(buffer) - 1; // Convert to zero-based index

    if (move == 7) {
        return 7; // Signal that the player wants to abandon
    }

    if (move < 0 || move >= PITS || game->board[game->status][move] == 0) {
        snprintf(buffer, sizeof(buffer), "Invalid move. Try again.\n");
        send(player->socket_fd, buffer, strlen(buffer), 0);
        return get_move(player, game); // Recursively call until a valid move is made
    }
    return move;
}


// check is someone won (got enough points)
int is_game_over(game_t* game) {
    if (game->player_scores[PLAYER1] >= 24 || game->player_scores[PLAYER2] >= 24) {
        return 1;
    }
    else {
        return 0;
    }
}


// Function to end the game, announce the winner, and clean up the game state
void end_game(game_t* game) {
    // Notify both players of the final score and the winner
    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "\nGame Over!\n");
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);

    snprintf(buffer, sizeof(buffer), "Player 1 Score: %d\n", game->player_scores[PLAYER1]);
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);

    snprintf(buffer, sizeof(buffer), "Player 2 Score: %d\n", game->player_scores[PLAYER2]);
    send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);

    if (game->player_scores[PLAYER1] > game->player_scores[PLAYER2]) {
        snprintf(buffer, sizeof(buffer), "Player 1 wins!\n");
    } else if (game->player_scores[PLAYER2] > game->player_scores[PLAYER1]) {
        snprintf(buffer, sizeof(buffer), "Player 2 wins!\n");
    } else {
        snprintf(buffer, sizeof(buffer), "It's a tie!\n");
    }

    send(game->player1->socket_fd, buffer, strlen(buffer), 0);
    send(game->player2->socket_fd, buffer, strlen(buffer), 0);

    free(game->player1);
    free(game->player2);
    free(game);
}


// Main game loop for each game instance
void play_game(game_t* game, client_t* client) {
    while (!is_game_over(game)) {
        // Determine which player's turn it is
        client_t* current_player = (game->status == PLAYER1_TURN) ? game->player1 : game->player2;

        if (current_player->socket_fd == client->socket_fd) {
            print_board(game);

            // Get the move from the current player
            int move = get_move(current_player, game);

            // check for abandonment
            if (move == 7) {
                game->status = (game->status == PLAYER1_TURN) ? PLAYER2_TURN : PLAYER1_TURN; // Opponent is winner
                break;
            }

            // Sow the seeds and proceed with the game
            sow_seeds(game, (game->status == PLAYER1_TURN) ? PLAYER1 : PLAYER2, move);

            // Switch turns
            game->status = (game->status == PLAYER1_TURN) ? PLAYER2_TURN : PLAYER1_TURN;


            // save the game state on each move
            save_game_to_file(game);

        }
    }

    // Game over, calculate the final scores and announce the result
    end_game(game);
}



void save_game_to_file(game_t* game) {
    FILE* file = fopen(FILENAME, "r");
    if (file == NULL) {
        printf("No saved games found.\n");
        return;
    }

    game_t* saved_games = malloc(sizeof(game_t) * MAX_SAVED_GAMES);
    int num_saved_games = 0;

    while (!feof(file)) {
        game_t* current_game = &saved_games[num_saved_games];
        current_game->player1 = malloc(sizeof(client_t));
        current_game->player2 = malloc(sizeof(client_t));

        if (fscanf(file, "Player 1: %s\n", current_game->player1->username) != 1) break;
        if (fscanf(file, "Player 2: %s\n", current_game->player2->username) != 1) break;
        fscanf(file, "Scores: %d %d\n", &current_game->player_scores[0], &current_game->player_scores[1]);

        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < PITS; k++) {
                if (fscanf(file, "%d", &current_game->board[j][k]) != 1) break;
            }
        }

        fscanf(file, "\n");
        fscanf(file, "Status: %d\n", &current_game->status);

        num_saved_games++;
    }
    fclose(file);

    int found = 0;
    for (int i = 0; i < num_saved_games; i++) {
        if ((strcmp(saved_games[i].player1->username, game->player1->username) == 0 &&
             strcmp(saved_games[i].player2->username, game->player2->username) == 0) ||
            (strcmp(saved_games[i].player1->username, game->player2->username) == 0 &&
             strcmp(saved_games[i].player2->username, game->player1->username) == 0)) {
            saved_games[i] = *game;
            found = 1;
            break;
        }
    }

    if (!found && num_saved_games < MAX_SAVED_GAMES) {
        saved_games[num_saved_games++] = *game;
    }

    file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Error opening file");
        free(saved_games);
        return;
    }

    for (int i = 0; i < num_saved_games; i++) {
        fprintf(file, "Player 1: %s\n", saved_games[i].player1->username);
        fprintf(file, "Player 2: %s\n", saved_games[i].player2->username);
        fprintf(file, "Scores: %d %d\n", saved_games[i].player_scores[0], saved_games[i].player_scores[1]);

        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < PITS; k++) {
                fprintf(file, "%d ", saved_games[i].board[j][k]);
            }
            fprintf(file, "\n");
        }

        fprintf(file, "Status: %d\n", saved_games[i].status);
    }

    fclose(file);
    free(saved_games);
}




