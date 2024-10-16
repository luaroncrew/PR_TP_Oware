CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = ./src
OBJ_DIR = obj
TARGET = bin/server

# Manually specify all source files
SRC = $(SRC_DIR)/server.c $(SRC_DIR)/game.c $(SRC_DIR)/commands.c # Add all your source files here

# Generate corresponding .o files in the OBJ_DIR
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Debugging output to show found source files
$(info Source files: $(SRC))
$(info Object files: $(OBJ))


server: src/server.c
	gcc -o $(TARGET) $(SRC) -pthread

# Clean up object files and executable
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

# Rule to run the program
run: $(TARGET)
	./$(TARGET)
