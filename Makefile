RM=rm
CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lasound -lm
SOURCES=main.c synth.c midi.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=pi-synth

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean: 
	$(RM) $(OBJECTS) $(EXECUTABLE)
