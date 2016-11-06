all: common.o
	clang++ -Wall main.cpp common.o -o crowd -std=c++11  -lsfml-graphics -lsfml-window -lsfml-system

common.o: ../common.cpp ../common.hpp
	clang++ -Wall -c ../common.cpp -o common.o -std=c++11
clean:
	rm crowd common.o
