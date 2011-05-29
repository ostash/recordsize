all: recordsize

recordsize:
	gcc -Wall -std=gnu99 -O2 -march=native -shared -fpic -I `gcc --print-file-name=plugin`/include -o recordsize.so recordsize.c
