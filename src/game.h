#ifndef GAME_H
#define GAME_H

#include "server.h"  // To include client_t structure
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PITS 6
#define SEEDS 4
#define PLAYER1 0
#define PLAYER2 1
#define BUFFER_SIZE 1024
#define SAVE_FILE_NAME "saved_games.txt"
#define NUM_PITS 12
#define MAX_SAVED_GAMES 10
#define FILENAME "saved_games.txt"

// Game status types
typedef enum {
    PLAYER1_TURN,
    PLAYER2_TURN,
    GAME_OVER
} game_status_t;

// Structure to hold the game's board, players, and state
typedef struct {
    int board[2][PITS];          // 2D array to represent the board (2 players, each with 6 pits)
    int player_scores[2];        // Scores for both players
    client_t* player1;           // Pointer to Player 1 (challenger)
    client_t* player2;           // Pointer to Player 2 (challenged)
    game_status_t status;        // Current status of the game
} game_t;

// Function declarations

// Initialize the board with seeds in each pit
void initialize_board(game_t* game);

// Print the current board state to a player's socket
void print_board(game_t* game);

// Check if a player has valid moves left
int has_valid_moves(int player);

// Capture seeds after a move is made
void capture_seeds(game_t* game, int player, int pit);

// Sow seeds starting from the chosen pit and handle game logic
void sow_seeds(game_t* game, int player, int pit);

// Get the player's move from their socket input
int get_move(client_t* player, game_t * game);

// Check if the game is over (i.e., no valid moves left)
int is_game_over(game_t* game);

// End the game, announce results, and reset the clients' game state
void end_game(game_t* game);

// Main game loop to play the game
void play_game(game_t* game, client_t * client);

void save_game_to_file(game_t* game);

#endif // GAME
