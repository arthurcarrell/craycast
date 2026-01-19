CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc
SDL_FLAGS = -lSDL3
LD_FLAGS = -lm
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c,obj/%.o,$(SRC))
TARGET = program

all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(SDL_FLAGS) $(LD_FLAGS) -o $(TARGET)

main.o: src/main.c $(HEAD)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/* $(TARGET)

run: $(TARGET)
	./$(TARGET)
