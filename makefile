all: test

test.o: test.cpp
	g++ `pkg-config --cflags opencv mysql_cppconn` -c test.cpp -o test.o

test: test.o
	g++ -o test test.o `pkg-config --libs opencv mysql_cppconn`  