ifeq ($(OS),Windows_NT)
	CC = x86_64-w64-mingw32-gcc
	LIBS = -lm -lraylib -lopengl32 -lgdi32 -lwinmm -lWs2_32
	OUT = vibro.exe
else
	CC = gcc
	LIBS = -lm -lraylib
	OUT = vibro
endif

OBJS = tinywav/tinywav.o globals.o synthesise.o octave.o note.o volume.o freq.o gui.o

all: executable
debug: CFLAGS += -g
debug: executable
executable: $(OBJS)
	$(CC) vibro.c $? -o $(OUT) -Wall $(CFLAGS) $(LIBS) -Llib

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o
	rm -f *.d