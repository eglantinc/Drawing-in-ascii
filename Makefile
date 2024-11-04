# Makefile pour canvascii.c

CC = gcc        

CFLAGS = -Wall -Wextra -std=c11

TARGET = canvascii
OBJECTS = canvascii.o 

.PHONY: all clean exec html

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJECTS): canvascii.c
	$(CC) -c $< -o $@ $(CFLAGS)

test:
	bats check.bats

clean:
	rm -f $(OBJECTS) $(TARGET)

html:
	pandoc -s README.md -o README.html --metadata title="TP1: Dessiner sur un canevas ASCII"

exec: $(TARGET)
	./$(TARGET)

