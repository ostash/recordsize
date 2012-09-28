all: recordsize

recordsize:
	gcc -Wall -std=gnu99 -O0 -g -march=native -shared -fpic -I `gcc --print-file-name=plugin`/include -o recordsize.so \
	FieldInfo.c recordsize.c
