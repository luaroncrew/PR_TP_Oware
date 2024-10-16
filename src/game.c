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
void print_board(game_t* game, client_t* client) {
    char buffer[BUFFER_SIZE];

    // Display board for Player 2's pits (top row)
    snprintf(buffer, sizeof(buffer), "\nPlayer 2 pits:\n");
    send(client->socket_fd, buffer, strlen(buffer), 0);
    for (int i = PITS - 1; i >= 0; i--) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER2][i]);
        send(client->socket_fd, buffer, strlen(buffer), 0);
    }

    // Display Player 1's pits (bottom row)
    snprintf(buffer, sizeof(buffer), "\n-----------------------\n");
    send(client->socket_fd, buffer, strlen(buffer), 0);
    for (int i = 0; i < PITS; i++) {
        snprintf(buffer, sizeof(buffer), "  %d ", game->board[PLAYER1][i]);
        send(client->socket_fd, buffer, strlen(buffer), 0);
    }
    snprintf(buffer, sizeof(buffer), "\nPlayer 1 pits:\n");
    send(client->socket_fd, buffer, strlen(buffer), 0);
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

    // Send a prompt to the player
    snprintf(buffer, sizeof(buffer), "Player %s, choose a pit (1-6): ", player->username);
    send(player->socket_fd, buffer, strlen(buffer), 0);

    // Wait for player's response
    recv(player->socket_fd, buffer, sizeof(buffer), 0);
    move = atoi(buffer) - 1; // Convert to zero-based index

    if (move < 0 || move >= PITS || game->board[game->status][move] == 0) {
        snprintf(buffer, sizeof(buffer), "Invalid move. Try again.\n");
        send(player->socket_fd, buffer, strlen(buffer), 0);
        return get_move(player, game); // Recursively call until a valid move is made
    }
    return move;
}

// Check if the game is over (i.e., no seeds to sow for either player)
int is_game_over(game_t* game) {
    return 0; // Placeholder for now
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
}

// Main game loop for each game instance
void play_game(game_t* game) {
    while (!is_game_over(game)) {
        // Determine which player's turn it is
        client_t* current_player = (game->status == PLAYER1_TURN) ? game->player1 : game->player2;

        // Print board for both players
        print_board(game, current_player);

        // Get the move from the current player
        int move = get_move(current_player, game);

        // Sow the seeds and proceed with the game
        sow_seeds(game, (game->status == PLAYER1_TURN) ? PLAYER1 : PLAYER2, move);

        // Switch turns
        game->status = (game->status == PLAYER1_TURN) ? PLAYER2_TURN : PLAYER1_TURN;
    }

    // Game over, calculate the final scores and announce the result
    end_game(game);
}
