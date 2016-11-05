all: common.o
	g++ -Wall main.cpp common.o -o crowd -std=c++11  -lsfml-graphics -lsfml-window -lsfml-system

common.o:
	g++ -Wall -c ../common.cpp -o common.o -std=c++11
clean:
	rm crowd
