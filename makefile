CC = gcc
CFLAGS = -Wall -Wextra -g
SDL_FLAGS = -lSDL3
LD_FLAGS = -lm
SRC = main.c utils.c
OBJ = $(SRC:.c=.o)
TARGET = program

all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(SDL_FLAGS) $(LD_FLAGS) -o $(TARGET)

main.o: main.c utils.h

clean:
	rm -f *.o $(TARGET)

run: $(TARGET)
	./$(TARGET)
