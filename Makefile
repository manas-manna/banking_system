# Makefile for Banking Management System

# Compiler
CC = gcc

# Directories for header files
INCLUDES = -I./functions -I./record-struct

# Libraries
LIBS = -lcrypt

# Output files
SERVER = server
CLIENT = client

# Source files
SERVER_SRC = server.c
CLIENT_SRC = client.c

# Compile all
all: $(SERVER) $(CLIENT)

# Compile server
$(SERVER): $(SERVER_SRC)
	$(CC) -o $(SERVER) $(SERVER_SRC) $(INCLUDES) $(LIBS)

# Compile client
$(CLIENT): $(CLIENT_SRC)
	$(CC) -o $(CLIENT) $(CLIENT_SRC) $(INCLUDES)

# Clean up object and binary files
clean:
	rm -f $(SERVER) $(CLIENT)

# Rebuild everything
rebuild: clean all