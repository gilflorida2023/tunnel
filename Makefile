# Makefile for SSH tunnel program
CC = gcc
CFLAGS = -Wall -Wextra -O0  -g3
TARGET = tunnel
SOURCES = tunnel.c
OBJECTS = $(SOURCES:.c=.o)
PROJECTSBIN = ~/projects/bin

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	cp $(TARGET) $(PROJECTSBIN)/$(TARGET)



# Phony targets
.PHONY: all clean

