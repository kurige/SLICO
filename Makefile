CC = g++

CFLAGS= -c -Wall

LINKFLAGS= -lil


all: main.out

main.out: main.cpp SLIC.cpp
	$(CC) $(LINKFLAGS) -o main.out main.cpp SLIC.cpp
