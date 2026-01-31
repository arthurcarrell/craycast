CC = gcc
CFLAGS = -Wall -Wextra -O0 -Isrc -fsanitize=address,undefined -g
SDL_FLAGS = -lSDL3
LD_FLAGS = -lm -fsanitize=address -fsanitize=undefined -g

# 1. Find all .c files in src AND any subdirectories (like map_parser)
SRC = $(shell find src -name "*.c")

# 2. This creates matching paths in the obj/ folder (e.g., obj/map_parser/parser.o)
OBJ = $(SRC:src/%.c=obj/%.o)

TARGET = program

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) $(SDL_FLAGS) $(LD_FLAGS) -o $(TARGET)

# 3. The rule to compile files and automatically create subfolders in obj/
obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj $(TARGET)

run: $(TARGET)
	./$(TARGET)
