SRC = src
OBJ = obj
INC = include
BIN = bin

EXECUTABLE = filesys
EXEC = $(BIN)/$(EXECUTABLE)

CC = gcc
CFLAGS = -Wall -Wextra -g -std=c99 -I$(INC)

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SRCS))

all: $(EXEC)

$(EXEC): $(OBJS) | $(BIN)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJ)/%.o: $(SRC)/%.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

run: $(EXEC)
	$(EXEC)

clean:
	rm -f $(OBJ)/*.o $(EXEC)
	rmdir $(OBJ) 2>/dev/null || true
	rmdir $(BIN) 2>/dev/null || true

.PHONY: all run clean