CFLAGS = -Wall -Wextra -Wpedantic -Wno-unused-but-set-variable -Wno-type-limits

ifeq ($(OS),Windows_NT)
	CC = x86_64-w64-mingw32-gcc
	LIBS = -lm -lraylib -lopengl32 -lgdi32 -lwinmm -lWs2_32
	OUT = vibro.exe
else
	CC = gcc
	LIBS = -lm -l:libraylib.a
	OUT = vibro
endif

OBJS = tinywav/tinywav.o util.o globals.o synthesise.o octave.o note.o volume.o freq.o gui.o sample.o instrument.o play_mode.o instrument_mode.o

all: $(OBJS)
	$(CC) vibro.c $? -o $(OUT) -Wall $(CFLAGS) $(LIBS) -Llib

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

debug: CFLAGS += -g -fanalyzer -fsanitize=address -fsanitize=undefined -fprofile-arcs -ftest-coverage
debug: all

clean:
	rm -f core.*
	rm -f *.gcno
	rm -f *.gcov
	rm -f *.o
	rm -f *.d