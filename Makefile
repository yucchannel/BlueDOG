# Compiler and flags
CC = gcc
CFLAGS = -Wall -g  # Include debugging information

# Target program name
TARGET = bluedog

# Source files
SRC = bluedog.c  # Add other source files here if needed

# Object files
OBJ = $(SRC:.c=.o)

# Installation directory
PREFIX = /usr/local/bin

# Default target (this is what 'make' will run by default)
all: $(TARGET)

# Compile the target program from object files
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install the compiled program to the system
install: $(TARGET)
	cp $(TARGET) $(PREFIX)

# Clean up object files and the target program
clean:
	rm -f $(TARGET) $(OBJ)

# Run the program with an input file
run: $(TARGET)
	./$(TARGET) program.bld  # You can pass any filename here for testing
