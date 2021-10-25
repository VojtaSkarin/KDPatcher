C_FLAGS=-std=c++17 -Wall -Wextra -Wpedantic

all: build

build: add.exe kdpatcher.exe
	add
	echo > build

kdpatcher: kdpatcher.exe
	echo > kdpatcher
	
kdpatcher.exe: kdpatcher.o
	g++ kdpatcher.o -o kdpatcher

kdpatcher.o: kdpatcher.cpp kdpatcher.hpp
	g++ -c $(C_FLAGS) kdpatcher.cpp -o kdpatcher.o

add: add.exe
	echo > add
	
add.exe: add.o
	g++ add.o -o add

add.o: add.cpp
	g++ -c $(C_FLAGS) add.cpp -o add.o

clean:
	del build add add.o add.exe kdpatcher kdpatcher.o kdpatcher.exe
