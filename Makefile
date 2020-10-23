# Set variables
CC = gcc
CFLAGS = -Wall -Werror
EXTRA = -lpthread -lm
TARGET = RRD
OTHER = pcb

# Make target if all is given
all: $(TARGET)

# Make target if target is given
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c $(OTHER).c $(EXTRA)

# Clean up files if clean is given
clean:
	rm $(TARGET)
