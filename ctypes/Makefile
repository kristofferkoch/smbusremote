
all: lib.so

lib.so: lib.o
	gcc -shared -o lib.so -fPIC lib.o

lib.o: lib.c
	gcc -g -O -Wall -fPIC -I. -c lib.c
