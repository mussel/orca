CC = g++
EXE = orca.exe

.PHONY: all clean

all: main
clean:
	rm -f *.o $(EXE)

main: main.cpp orca.o
	$(CC) main.cpp orca.o -lm -o $(EXE)

orca.o: orca.cpp orca.h
	$(CC) -c orca.cpp 
