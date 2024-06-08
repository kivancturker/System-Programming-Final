# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -I$(INCLUDE_DIR) -Wall -pthread -std=gnu99

# Directories
SRC_DIR = src
OBJ_DIR = obj
INCLUDE_DIR = include

# Executables
SERVER_EXE = PideShop
CLIENT_EXE = HungryVeryMuch

# Source files
SERVER_SRC = $(SRC_DIR)/server.c $(SHARED_SRC) $(SRC_DIR)/manager.c $(SRC_DIR)/logger.c $(SRC_DIR)/cook.c
CLIENT_SRC = $(SRC_DIR)/client.c $(SHARED_SRC)
SHARED_SRC = $(SRC_DIR)/myutil.c $(SRC_DIR)/sockconn.c

# Object files
SERVER_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SERVER_SRC))
CLIENT_OBJ = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(CLIENT_SRC))

# Default target
all: $(SERVER_EXE) $(CLIENT_EXE)

# Build server executable
$(SERVER_EXE): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Build client executable
$(CLIENT_EXE): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Run server
run_server: $(SERVER_EXE)
	./$(SERVER_EXE) 12345 4 6 1 &

# Run client
run_client: $(CLIENT_EXE)
	./$(CLIENT_EXE) 12345 50 10 20

# Run both server and client
run: run_server run_client

# Clean object files, executables, and log.txt
clean:
	rm -rf $(OBJ_DIR)/*.o $(SERVER_EXE) $(CLIENT_EXE) log.txt

# Phony targets
.PHONY: all clean run_server run_client run