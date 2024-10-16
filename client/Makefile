CC = gcc
CFLAGS = -Wall -Wextra
SRC = client.c               # Client source file
TARGET = client               # Output executable name

# Default rule to build the target
all: $(TARGET)

# Rule to compile the target executable
$(TARGET): $(SRC)
	gcc $(CFLAGS) client.c -o client

# Clean up the executable
clean:
	rm -f $(TARGET)

# Rule to run the program
run: $(TARGET)
	./$(TARGET)
