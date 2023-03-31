ifeq ($(OS),Windows_NT)
CC = x86_64-w64-mingw32-gcc
LIBS = -lm -lraylib -lopengl32 -lgdi32 -lwinmm -lWs2_32
OUT = keyboard.exe
CFLAGS += -I. -L.
else  # either Linux or MacOS, but for now I only support the former
CC = gcc
LIBS = -lm -lraylib_linux
OUT = keyboard
CFLAGS += -L.
endif

all: executable
debug: CFLAGS += -g
debug: executable

executable:
	$(CC) keyboard.c tinywav/tinywav.c -o $(OUT) -Wall $(CFLAGS) $(LIBS)
