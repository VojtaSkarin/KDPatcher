all: kdpatcher

kdpatcher:
	g++ -std=c++17 -Wall -Wextra -Wpedantic kdpatcher.cpp -o kdpatcher
	add

add:
	g++ -std=c++17 -Wall -Wextra -Wpedantic add.cpp -o add