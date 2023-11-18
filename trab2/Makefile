.POSIX:
.SUFFIXES:
CC = gcc
CFLAGS = -W
LDLIBS = -lm
GTKFLAGS = `pkg-config --cflags gtk4`
GTKLIBS = `pkg-config --libs gtk4`

main: main.c 
	$(CC) $(GTKFLAGS) -o main main.c $(GTKLIBS) $(LDLIBS)

test: test.c 
	$(CC) -o test test.c $(LDLIBS)

clean:
	rm main 

