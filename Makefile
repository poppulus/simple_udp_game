#Check for OS
ifeq ($(OS), Windows_NT)
	uname_S := Windows
else 
	uname_S := $(shell uname -s)
endif

ifeq ($(uname_S), Windows)
	target = puck.exe
endif
ifeq ($(uname_S), Linux)
	target = puck
endif

#OBJS specifies which files to compile as part of the project
OBJS = main.c util.c SDL_FontCache.c net.c

#CC specifies which compiler we're using
CC = gcc

#COMPILER_FLAGS specifies the additional compilation options we're using
COMPILER_FLAGS = -Wall -g

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2_net

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = puck

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
