CC = x86_64-w64-mingw32-gcc
LIBS = -lm -lraylib -lopengl32 -lgdi32 -lwinmm -lWs2_32

all:
	$(CC) keyboard.c tinywav/tinywav.c -o keyboard.exe -I. -L. -Wall $(LIBS)

joystick:
	$(CC) keyboard-joystick.c tinywav/tinywav.c -o keyboard-joystick.exe -I. -L. -Wall $(LIBS)
