ifeq ($(OS),Windows_NT)
	CC = x86_64-w64-mingw32-gcc
	LIBS = -lm -lraylib -lopengl32 -lgdi32 -lwinmm -lWs2_32
	OUT = vibro.exe
	CFLAGS += -Llib
else
	CC = gcc
	LIBS = -lm -lraylib
	OUT = vibro
	CFLAGS += -Llib
endif

all: executable
debug: CFLAGS += -g
debug: executable

globals.o: globals.c
synthesise.o: synthesise.c
octave.o: octave.c
keys.o: keys.c
volume.o: volume.c
freq.o: freq.c
input.o: input.c
gui.o: gui.c
tinywav/tinywav.o: tinywav/tinywav.c

executable: tinywav/tinywav.o octave.o keys.o volume.o freq.o input.o globals.o gui.o synthesise.o
	$(CC) vibro.c $? -o $(OUT) -Wall $(CFLAGS) $(LIBS)