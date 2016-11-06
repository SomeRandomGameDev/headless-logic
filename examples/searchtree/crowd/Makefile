all: common.o
	clang++ -Wall -g main.cpp common.o -o crowd -std=c++11  -lsfml-graphics -lsfml-window -lsfml-system

common.o: ../common.cpp ../common.hpp /usr/local/include/headless-logic/searchtree.hpp
	clang++ -Wall -g -c ../common.cpp -o common.o -std=c++11
clean:
	rm crowd common.o
