# Simple Makefile for Linux/Mac
CC = gcc
CFLAGS = -g -O2 -Wall -std=c99
ifeq ($(BUILD),opengles)
	CFLAGS += -DOPENGLES
	SRC = main.c embededFont.c glad/es/glad.c
else
	CFLAGS += -DOPENGL
	SRC = main.c embededFont.c glad/glad.c
endif
OBJ = $(SRC:.c=.o)
TARGET = shadertoy

# Includes: current dir for embededFont, glad, and SDL2
INCLUDES = -I. $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs)

all: $(TARGET)

opengles:
	$(MAKE) BUILD=opengles

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
