CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11
LDFLAGS =

CLIENT_SRC = client.cpp
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
CLIENT_EXEC = client

SERVER_SRC = server.cpp
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
SERVER_EXEC = server

all: $(CLIENT_EXEC) $(SERVER_EXEC)

$(CLIENT_EXEC): $(CLIENT_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

$(SERVER_EXEC): $(SERVER_OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_EXEC) $(SERVER_EXEC)
