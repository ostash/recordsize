all: recordsize

recordsize:
	g++ -Wall -std=gnu++98 -O2 -march=native -shared -fpic -I `g++ --print-file-name=plugin`/include -o recordsize.so recordsize.cpp
