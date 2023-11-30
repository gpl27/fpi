.POSIX:
.SUFFIXES:
CC = g++
LDLIBS =
OCVFLAGS = `pkg-config --cflags opencv4`
OCVLIBS = `pkg-config --libs opencv4`

main: main.cpp
	$(CC) $(OCVFLAGS) -o main main.cpp $(OCVLIBS) $(LDLIBS)

clean:
	rm main 

