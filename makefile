all: test

test.o: test.cpp
	g++ `pkg-config --cflags opencv` -c test.cpp -o test.o

test: test.o
	g++ -o test test.o `pkg-config --libs opencv`  