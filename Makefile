all:
	gcc -std=c11 -Wall -shared -fPIC -Ilua-5.3.2/src -Llua-5.3.2/src -llua -o scriptrunner.so scriptrunner.c
