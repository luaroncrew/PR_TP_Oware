#include <stdio.h>

#define PITS 6
#define SEEDS 4
#define PLAYER1 0
#define PLAYER2 1

int board[2][PITS];  // 2 rows (players), each with 6 pits
int player_scores[2] = {0, 0}; // scores for each player

// Initialize the board with the standard setup (4 seeds per pit)
void initialize_board() {
    for (int i = 0; i < PITS; i++) {
        board[PLAYER1][i] = SEEDS;
        board[PLAYER2][i] = SEEDS;
    }
}

// Print the current state of the board
void print_board() {
    printf("\nPlayer 2 pits:\n");
    for (int i = PITS - 1; i >= 0; i--) {
        printf("  %d ", board[PLAYER2][i]);
    }
    printf("\n-----------------------\n");
    for (int i = 0; i < PITS; i++) {
        printf("  %d ", board[PLAYER1][i]);
    }
    printf("\nPlayer 1 pits:\n");
}

// Check if the player has valid moves (i.e., any seeds in their pits)
int has_valid_moves(int player) {
    for (int i = 0; i < PITS; i++) {
        if (board[player][i] > 0) {
            return 1;
        }
    }
    return 0;
}

// Capture seeds after sowing
void capture_seeds(int player, int pit) {
    int opponent = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    while (pit >= 0 && (board[opponent][pit] == 2 || board[opponent][pit] == 3)) {
        player_scores[player] += board[opponent][pit];
        board[opponent][pit] = 0;
        pit--;
    }
}

// Sow the seeds from the chosen pit
void sow_seeds(int player, int pit) {
    int seeds = board[player][pit];
    board[player][pit] = 0;

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

        board[current_player][current_pit]++;
        seeds--;
        current_pit++;
    }

    // Check if the last seed was sown in the opponent's pits for capturing
    if (current_player != player && current_pit > 0) {
        capture_seeds(player, current_pit - 1);
    }
}

// Get the player's move
int get_move(int player) {
    int move;
    do {
        printf("Player %d, choose a pit (1-6): ", player + 1);
        scanf("%d", &move);
        move--; // Convert to zero-based index
        if (move < 0 || move >= PITS || board[player][move] == 0) {
            printf("Invalid move. Try again.\n");
        }
    } while (move < 0 || move >= PITS || board[player][move] == 0);
    return move;
}

// Check if the game is over (i.e., no seeds to sow)
int is_game_over() {
    return !has_valid_moves(PLAYER1) || !has_valid_moves(PLAYER2);
}

// Main game loop
void play_game() {
    int current_player = PLAYER1;
    initialize_board();

    while (!is_game_over()) {
        print_board();
        int move = get_move(current_player);
        sow_seeds(current_player, move);
        current_player = (current_player == PLAYER1) ? PLAYER2 : PLAYER1;
    }

    // Final score
    printf("\nGame Over!\n");
    printf("Player 1 Score: %d\n", player_scores[PLAYER1]);
    printf("Player 2 Score: %d\n", player_scores[PLAYER2]);

    if (player_scores[PLAYER1] > player_scores[PLAYER2]) {
        printf("Player 1 wins!\n");
    } else if (player_scores[PLAYER2] > player_scores[PLAYER1]) {
        printf("Player 2 wins!\n");
    } else {
        printf("It's a tie!\n");
    }
}

int main() {
    play_game();
    return 0;
}
