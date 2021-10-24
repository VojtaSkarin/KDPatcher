C_FLAGS=-std=c++17 -Wall -Wextra -Wpedantic

all: build

build: add.exe kdpatcher.exe
	add

kdpatcher.exe: kdpatcher.o
	g++ kdpatcher.o -o kdpatcher

kdpatcher.o: kdpatcher.cpp kdpatcher.hpp
	g++ -c $(C_FLAGS) kdpatcher.cpp -o kdpatcher.o

add.exe: add.o
	g++ add.o -o add

add.o: add.cpp
	g++ -c $(C_FLAGS) add.cpp -o add.o

clean:
	del add.o add.exe kdpatcher.o kdpatcher.exe
